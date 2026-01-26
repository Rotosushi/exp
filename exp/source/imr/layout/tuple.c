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

#include "imr/layout/tuple.h"
#include "env/layout_interner.h"
#include "imr/type.h"
#include "support/allocation.h"
#include "support/arithmetic.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/panic.h"
#include "support/unreachable.h"

static void layout_tuple_initialize(LayoutTuple *restrict layout) {
    EXP_ASSERT(layout != NULL);
    layout->primary  = layout_primary(0, 0);
    layout->length   = 0;
    layout->capacity = 0;
    layout->buffer   = NULL;
}

static void layout_tuple_compute_size(LayoutTuple *restrict layout) {
    EXP_ASSERT(layout != NULL);
    u64 size = 0;
    for (u32 index = 0; index < layout->length; ++index) {
        Layout const *element_layout = layout->buffer[index];
        u64           element_size   = layout_size_of(element_layout);

        if (add_u64(size, element_size, &size)) { PANIC("size overflow"); }
    }

    layout->primary.size = size;
}

static void layout_tuple_compute_alignment(LayoutTuple *restrict layout) {
    EXP_ASSERT(layout != NULL);
    u64 alignment = 0;
    for (u32 index = 0; index < layout->length; ++index) {
        Layout const *element_layout    = layout->buffer[index];
        u64           element_alignment = layout_align_of(element_layout);

        if (element_alignment > alignment) { alignment = element_alignment; }
    }

    layout->primary.alignment = alignment;
}

static bool layout_tuple_full(LayoutTuple const *restrict layout) {
    return (layout->length + 1) >= layout->capacity;
}

static void layout_tuple_grow(LayoutTuple *restrict layout) {
    Growth_u32 g = array_growth_u32(layout->capacity, sizeof(*layout->buffer));
    layout->buffer   = reallocate(layout->buffer, g.alloc_size);
    layout->capacity = g.new_capacity;
}

static void layout_tuple_append(LayoutTuple *restrict layout,
                                Layout const *element) {
    if (layout_tuple_full(layout)) { layout_tuple_grow(layout); }
    layout->buffer[layout->length++] = element;
}

static void layout_tuple_append_element(LayoutTuple *restrict layout,
                                        Layout const *element,
                                        Layout const *next,
                                        LayoutInterner *restrict interner) {
    layout_tuple_append(layout, element);
    if (next == NULL) { return; }

    u64 element_alignment = layout_align_of(element);
    u64 next_alignment    = layout_align_of(next);
    // #NOTE:
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
    if (next_alignment <= element_alignment) { return; }

    u64           padding_length = next_alignment - element_alignment;
    Layout const *padding =
        layout_interner_get_padding(interner, padding_length);
    layout_tuple_append(layout, padding);
}

void layout_tuple_create(LayoutTuple *restrict layout,
                         TypeTuple const *restrict tuple,
                         LayoutInterner *restrict interner) {
    EXP_ASSERT(layout != NULL);
    EXP_ASSERT(tuple != NULL);
    EXP_ASSERT(interner != NULL);
    Type const   *element_type   = NULL;
    Type const   *next_type      = NULL;
    Layout const *element_layout = NULL;
    Layout const *next_layout    = NULL;
    for (u32 index = 0; index < tuple->length; ++index) {
        element_type   = tuple->types[index];
        element_layout = layout_interner_layout_of(interner, element_type);

        if (index < (tuple->length - 1)) {
            next_type   = tuple->types[index + 1];
            next_layout = layout_interner_layout_of(interner, next_type);
        } else {
            next_type   = NULL;
            next_layout = NULL;
        }

        layout_tuple_append_element(
            layout, element_layout, next_layout, interner);
    }

    layout_tuple_compute_size(layout);
    layout_tuple_compute_alignment(layout);
}

void layout_tuple_destroy(LayoutTuple *restrict layout) {
    EXP_ASSERT(layout != NULL);
    deallocate(layout->buffer);
    layout_tuple_initialize(layout);
}

u64 layout_tuple_get_element_offset(LayoutTuple const *restrict layout, u32 n) {
    EXP_ASSERT(layout != NULL);

    u64 offset = 0;
    for (u32 index = 0, count = 0; index < layout->length; ++index, ++count) {
        Layout const *element = layout->buffer[index];
        switch (element->kind) {
        case LAYOUT_KIND_TUPLE:
            offset += element->data.tuple.primary.size;
            count += 1;
            break;

        case LAYOUT_KIND_PRIMARY:
            offset += element->data.primary.size;
            count += 1;
            break;

        case LAYOUT_KIND_PADDING: offset += element->data.padding; break;

        default: EXP_UNREACHABLE();
        }

        if (count >= n) { break; }
    }

    return offset;
}
