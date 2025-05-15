/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "codegen/x86/imr/layout.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/unreachable.h"

/*
 * The x86 assembly layout of a tuple is rather complex.
 * we have to take into account:
 * 1: The type of each element in the tuple.
 * 2: The initializer of each element in the tuple, (where applicable)
 * 3: The size and alignment of each element in the tuple
 * 4: The size and alignment of each pair of elements within the tuple.
 *
 * The layout in simple terms in just space for each successive element
 * within the tuple. So you would think we could just place each successive
 * element one after another right down into the assembly. This almost works,
 * except that we might not have initializers for each element, and in this
 * case, we just want to zero out the space for those elements, and since we
 * are zeroing out uninitialized elements, we can combine the .zero directives
 * together by simply adding up the sizes. So long as the allocations are
 * adjacent in memory that is. This still isn't enough however, as we need to
 * ensure that accessing the memory of specific elements of the tuple is
 * aligned. This means we must add padding between elements which increase in
 * alignment. For instance, two byte allocations can be placed adjacent to
 * eachother, But a byte allocation followed by a u32 allocation must have 3
 * bytes of padding placed between them, to ensure that the u32 allocation is
 * aligned to a 4 byte boundary. And this fact has to be taken into
 * consideration even when computing uninitialized elements.
 *
 * Now, this is all in the general case, as we could allow for the rearrangement
 * of elements within the tuple to reduce the size of the overall structure,
 * For instance, (u8, u32, u8) layed out as is, must be larger due to padding
 * than (u8, u8, u32), However we can allow the user to be blissfully unaware of
 * this fact and handle the difference under the hood of the compiler. This
 * works best, in my opinion, with boolean type elements. as if we can
 * automatically turn these into bitfields within the padding of the struct,
 * this is a huge win in terms of space allocated for a structure. The future
 * design is to build named field structure types on top of the machinery used
 * for tuples, So the user can use one or the other when they want, and we
 * can support both without having to repeat the redundant parts of the
 * implementation. This is not always desired however, when considering an
 * embedded context, sometimes one needs to express some arrangement of bits and
 * bytes which is specified by the hardware manufacturer. In this case one needs
 * full control over how the memory is layed out, and the C style becomes
 * favorable. esp. with bitfields and union types.
 *
 * Lets say, for now, that we follow in C's footsteps, as that semantic is
 * necessary in either case. It's just nice to allow the compiler to reason
 * about packing the structure more efficiently, instead of requiring it to be
 * programmer controlled.
 * But my intention for the future is to follow in Zig's footsteps, offloading
 * the really tedious and hard to get right things into the compiler, with the
 * option to have full control when necessary.
 */

static u64 x86_tuple_layout_size_of(x86_TupleLayout *restrict tuple) {
    u64 size = 0;
    for (u32 index = 0; index < tuple->size; ++index) {
        x86_Layout const *element      = tuple->buffer[index];
        u64               element_size = x86_layout_size_of(element);

        if (__builtin_add_overflow(size, element_size, &size)) {
            PANIC("integer overflow");
        }
    }
    tuple->size = size;
    return size;
}

static u64 x86_tuple_layout_align_of(x86_TupleLayout *restrict tuple) {
    u64 alignment = 0;
    for (u32 index = 0; index < tuple->size; ++index) {
        x86_Layout const *element           = tuple->buffer[index];
        u64               element_alignment = x86_layout_align_of(element);

        if (element_alignment > alignment) { alignment = element_alignment; }
    }
    tuple->alignment = alignment;
    return alignment;
}

static bool x86_tuple_layout_full(x86_TupleLayout const *restrict layout) {
    return (layout->length + 1) >= layout->capacity;
}

static void x86_tuple_layout_grow(x86_TupleLayout *restrict layout) {
    Growth_u32 g = array_growth_u32(layout->capacity, sizeof(*layout->buffer));
    layout->buffer   = reallocate(layout->buffer, g.alloc_size);
    layout->capacity = g.new_capacity;
}

static void x86_tuple_layout_append_element(x86_TupleLayout *restrict layout,
                                            x86_Layout const *element) {
    if (x86_tuple_layout_full(layout)) { x86_tuple_layout_grow(layout); }
    layout->buffer[layout->length++] = element;
}

static void x86_layout_create_padding(x86_Layout *restrict layout, u64 padding);
static void x86_layout_list_append(x86_LayoutList *restrict list,
                                   Type const *type,
                                   x86_Layout *layout);
static x86_LayoutListElement *
x86_layout_list_lookup_padding(x86_LayoutList *restrict list, u64 padding);

static void x86_tuple_layout_append(x86_TupleLayout *restrict tuple,
                                    x86_Layout const *layout,
                                    x86_Layout const *next,
                                    x86_Layouts *restrict layouts) {
    x86_tuple_layout_append_element(tuple, layout);
    if (next == NULL) { return; }

    u64 layout_alignment = x86_layout_align_of(layout);
    u64 next_alignment   = x86_layout_align_of(next);
    // #NOTE: The reasoning here is:
    // if the alignment requirements of the next element of the tuple
    // are greater than the alignment requirements of this element,
    // placing the next element down immediately after this element will
    // cause the next element to be misaligned in memory. Therefore we
    // need padding between this element and the next element.
    // The amount of padding is the difference in alignment. because that
    // is the number of bytes required to get to an aligned point.
    // assuming we are starting aligned. Which based on how x86_codegen
    // is written, we ask the assembler to align each symbol. So we are
    // relying on the assembler to start the layout at an aligned point.
    if (next_alignment <= layout_alignment) { return; }

    u64 padding = next_alignment - layout_alignment;

    // #NOTE:
    // since most alignments are (1, 2, 4, 8), most paddings are
    // (1, 2, 3, 4, 7). We can reuse those x86_Layout allocations
    // by looking through the existing paddings before we add a new one.
    // This should be a win in terms of memory allocated by the program.
    x86_LayoutListElement *existing =
        x86_layout_list_lookup_padding(&layouts->paddings, padding);
    if (existing != NULL) {
        x86_tuple_layout_append_element(tuple, existing->layout);
        return;
    }

    x86_Layout *padding_layout = allocate(sizeof(x86_Layout));
    x86_layout_create_padding(padding_layout, padding);
    // #NOTE: #WARNING !!!WE ARE PASSING ~NULL~ HERE FOR THE TYPE!!!
    // This is only fine because we are NEVER going to lookup a padding
    // based on it's type. We are only using a LayoutList because it is
    // convenient to reuse what is already written, and a LayoutList is
    // already an owner of a list of layout allocations.
    x86_layout_list_append(&layouts->paddings, NULL, padding_layout);
    x86_tuple_layout_append_element(tuple, padding_layout);
}

static void x86_tuple_layout_create(x86_TupleLayout *restrict layout,
                                    TupleType const *restrict tuple,
                                    x86_Layouts *restrict layouts) {
    Type const       *element        = NULL;
    Type const       *next           = NULL;
    x86_Layout const *element_layout = NULL;
    x86_Layout const *next_layout    = NULL;
    for (u32 index = 0; index < tuple->size; ++index) {
        if (index < (tuple->size - 1)) {
            element        = tuple->types[index];
            next           = tuple->types[index + 1];
            element_layout = x86_layouts_layout_of_type(layouts, element);
            next_layout    = x86_layouts_layout_of_type(layouts, next);
        } else {
            element        = tuple->types[index];
            next           = NULL;
            element_layout = x86_layouts_layout_of_type(layouts, element);
            next_layout    = NULL;
        }
        x86_tuple_layout_append(layout, element_layout, next_layout, layouts);
    }

    x86_tuple_layout_size_of(layout);
    x86_tuple_layout_align_of(layout);
}

static void x86_tuple_layout_destroy(x86_TupleLayout *restrict layout) {
    deallocate(layout->buffer);
    layout->size     = 0;
    layout->capacity = 0;
    layout->buffer   = NULL;
}

static void
x86_layout_create_scalar(x86_Layout *restrict layout, u8 size, u8 alignment) {
    layout->kind = X86_LAYOUT_KIND_SCALAR;
    layout->data.scalar =
        (x86_ScalarLayout){.size = size, .alignment = alignment};
}

static void x86_layout_create_padding(x86_Layout *restrict layout,
                                      u64 padding) {
    layout->kind         = X86_LAYOUT_KIND_PADDING;
    layout->data.padding = padding;
}

static void x86_layout_create_tuple(x86_Layout *restrict layout,
                                    TupleType const *tuple,
                                    x86_Layouts *restrict layouts) {
    layout->kind = X86_LAYOUT_KIND_TUPLE;
    x86_tuple_layout_create(&layout->data.tuple, tuple, layouts);
}

static void x86_layout_destroy(x86_Layout *restrict layout) {
    switch (layout->kind) {
    case X86_LAYOUT_KIND_TUPLE:
        x86_tuple_layout_destroy(&layout->data.tuple);
        break;
    // Scalar layouts do not dynamically allocate
    // Padding layouts do not dynamically allocate
    default: break;
    }
}

u64 x86_layout_size_of(x86_Layout const *restrict layout) {
    exp_assert(layout != NULL);
    switch (layout->kind) {
    case X86_LAYOUT_KIND_SCALAR: return layout->data.scalar.size;
    case X86_LAYOUT_KIND_TUPLE:  return layout->data.tuple.size;
    default:                     EXP_UNREACHABLE();
    }
}

u64 x86_layout_align_of(x86_Layout const *restrict layout) {
    exp_assert(layout != NULL);
    switch (layout->kind) {
    case X86_LAYOUT_KIND_SCALAR: return layout->data.scalar.alignment;
    case X86_LAYOUT_KIND_TUPLE:  return layout->data.tuple.alignment;
    default:                     EXP_UNREACHABLE();
    }
}

static void x86_layout_list_create(x86_LayoutList *restrict list) {
    list->length   = 0;
    list->capacity = 0;
    list->buffer   = NULL;
}

static void x86_layout_list_destroy(x86_LayoutList *restrict list) {
    for (u32 index = 0; index < list->length; ++index) {
        x86_Layout *layout = list->buffer[index].layout;
        x86_layout_destroy(layout);
        deallocate(layout);
    }
    deallocate(list->buffer);
    x86_layout_list_create(list);
}

static bool x86_layout_list_full(x86_LayoutList const *restrict list) {
    return (list->length + 1) >= list->capacity;
}

static void x86_layout_list_grow(x86_LayoutList *restrict list) {
    Growth_u32 g   = array_growth_u32(list->capacity, sizeof(*list->buffer));
    list->buffer   = reallocate(list->buffer, g.alloc_size);
    list->capacity = g.new_capacity;
}

static x86_LayoutListElement *
x86_layout_list_lookup(x86_LayoutList *restrict list, Type const *type) {
    for (u32 index = 0; index < list->length; ++index) {
        x86_LayoutListElement *element = list->buffer + index;
        if (type == element->type) { return element; }
    }
    return NULL;
}

static x86_LayoutListElement *
x86_layout_list_lookup_padding(x86_LayoutList *restrict list, u64 padding) {
    for (u32 index = 0; index < list->length; ++index) {
        x86_LayoutListElement *element = list->buffer + index;
        exp_assert(element->type == NULL);
        exp_assert(element->layout->kind == X86_LAYOUT_KIND_PADDING);
        if (padding == element->layout->data.padding) { return element; }
    }
    return NULL;
}

static void x86_layout_list_append(x86_LayoutList *restrict list,
                                   Type const *type,
                                   x86_Layout *layout) {
    if (x86_layout_list_full(list)) { x86_layout_list_grow(list); }

    x86_LayoutListElement *element = list->buffer + list->length++;
    element->type                  = type;
    element->layout                = layout;
}

void x86_layouts_create(x86_Layouts *restrict layouts) {
    exp_assert(layouts != NULL);
    // #NOTE: single byte objects do not
    // have an alignment specified by gcc or
    // clang. I believe this is
    // because we are aligning all other
    // objects, so when we allocate the byte
    // we are guaranteed to be at a location counter
    // that is valid for a single byte.
    // Though I don't see the harm in specifying an
    // alignment. as it becomes a noop in the assembler.
    // The only thing is it will cause the assembler to
    // run marginally slower for each noop alignment.
    // for other objects, the alignment is often equal
    // to their size. quad-words are 8 bytes, and their
    // alignment is 8. double-words are 4 bytes, and their
    // alignment is 4. words are 2 bytes, alignment is 2 bytes.
    // string literals are align 8.
    x86_layout_create_scalar(&layouts->nil, 1, 1);
    x86_layout_create_scalar(&layouts->bool_, 1, 1);
    x86_layout_create_scalar(&layouts->u8_, 1, 1);
    x86_layout_create_scalar(&layouts->u16_, 2, 2);
    x86_layout_create_scalar(&layouts->u32_, 4, 4);
    x86_layout_create_scalar(&layouts->u64_, 8, 8);
    x86_layout_create_scalar(&layouts->i8_, 1, 1);
    x86_layout_create_scalar(&layouts->i16_, 2, 2);
    x86_layout_create_scalar(&layouts->i32_, 4, 4);
    x86_layout_create_scalar(&layouts->i64_, 8, 8);
    x86_layout_list_create(&layouts->paddings);
    x86_layout_list_create(&layouts->tuples);
}

void x86_layouts_destroy(x86_Layouts *restrict layouts) {
    exp_assert(layouts != NULL);
    x86_layout_list_destroy(&layouts->paddings);
    x86_layout_list_destroy(&layouts->tuples);
}

x86_Layout const *x86_layouts_layout_of_type(x86_Layouts *restrict layouts,
                                             Type const *type) {
    exp_assert(layouts != NULL);
    exp_assert(type != NULL);
    switch (type->kind) {
    case TYPE_KIND_NIL:  return &layouts->nil;
    case TYPE_KIND_BOOL: return &layouts->bool_;
    case TYPE_KIND_U8:   return &layouts->u8_;
    case TYPE_KIND_U16:  return &layouts->u16_;
    case TYPE_KIND_U32:  return &layouts->u32_;
    case TYPE_KIND_U64:  return &layouts->u64_;
    case TYPE_KIND_I8:   return &layouts->i8_;
    case TYPE_KIND_I16:  return &layouts->i16_;
    case TYPE_KIND_I32:  return &layouts->i32_;
    case TYPE_KIND_I64:  return &layouts->i64_;

    case TYPE_KIND_TUPLE: {
        x86_LayoutListElement *existing =
            x86_layout_list_lookup(&layouts->tuples, type);
        if (existing != NULL) { return existing->layout; }

        x86_Layout *layout = allocate(sizeof(x86_Layout));
        x86_layout_create_tuple(layout, &type->tuple_type, layouts);
        x86_layout_list_append(&layouts->tuples, type, layout);
        return layout;
    }

    // we don't compute a layout for functions, as the size is not
    // known until after they are assembled, though their alignment
    // is 8. (machine word size aligned)
    default: EXP_UNREACHABLE();
    }
}
