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

#include "env/layout_interner.h"
#include "env/context.h"
#include "imr/layout.h"
#include "imr/type.h"
#include "imr/type/composite.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/unreachable.h"

static void layout_list_create(LayoutList *restrict list) {
    list->length   = 0;
    list->capacity = 0;
    list->buffer   = NULL;
}

static void layout_list_destroy(LayoutList *restrict list) {
    for (u32 index = 0; index < list->length; ++index) {
        Layout *layout = list->buffer[index].layout;
        layout_destroy(layout);
        deallocate(layout);
    }

    deallocate(list->buffer);
    layout_list_create(list);
}

static bool layout_list_full(LayoutList const *restrict list) {
    return (list->length + 1) >= list->capacity;
}

static void layout_list_grow(LayoutList *restrict list) {
    Growth_u32 g   = array_growth_u32(list->capacity, sizeof(*list->buffer));
    list->buffer   = reallocate(list->buffer, g.alloc_size);
    list->capacity = g.new_capacity;
}

static LayoutListElement *layout_list_lookup(LayoutList const *restrict list,
                                             Type const *type) {
    for (u32 index = 0; index < list->length; ++index) {
        LayoutListElement *element = list->buffer + index;
        if (type_equality(type, element->type)) { return element; }
    }
    return NULL;
}

static LayoutListElement *
layout_list_lookup_padding(LayoutList const *restrict list,
                           u64 padding_length) {
    for (u32 index = 0; index < list->length; ++index) {
        LayoutListElement *element = list->buffer + index;
        exp_assert(element->type == NULL);
        exp_assert(element->layout->kind == LAYOUT_KIND_PADDING);
        if (padding_length == element->layout->data.padding) { return element; }
    }
    return NULL;
}

static void layout_list_append(LayoutList *restrict list,
                               Type const *type,
                               Layout     *layout) {
    if (layout_list_full(list)) { layout_list_grow(list); }
    LayoutListElement *element = list->buffer + list->length++;
    element->type              = type;
    element->layout            = layout;
}

void layout_interner_create(LayoutInterner *restrict interner,
                            Context *restrict context) {
    layout_create_primary(
        &interner->nil, context_layout_of_primary(context, type_primary_nil()));
    layout_create_primary(
        &interner->bool_,
        context_layout_of_primary(context, type_primary_bool()));
    layout_create_primary(
        &interner->u8_, context_layout_of_primary(context, type_primary_u8()));
    layout_create_primary(
        &interner->u16_,
        context_layout_of_primary(context, type_primary_u16()));
    layout_create_primary(
        &interner->u32_,
        context_layout_of_primary(context, type_primary_u32()));
    layout_create_primary(
        &interner->u64_,
        context_layout_of_primary(context, type_primary_u64()));
    layout_create_primary(
        &interner->i8_, context_layout_of_primary(context, type_primary_i8()));
    layout_create_primary(
        &interner->i16_,
        context_layout_of_primary(context, type_primary_i16()));
    layout_create_primary(
        &interner->i32_,
        context_layout_of_primary(context, type_primary_i32()));
    layout_create_primary(
        &interner->i64_,
        context_layout_of_primary(context, type_primary_i64()));
    layout_create_function(&interner->function,
                           context_layout_of_function(context));

    layout_list_create(&interner->padding);
    layout_list_create(&interner->tuple);
}

void layout_interner_destroy(LayoutInterner *restrict interner) {
    layout_list_destroy(&interner->padding);
    layout_list_destroy(&interner->tuple);
}

Layout const *layout_interner_get_padding(LayoutInterner *restrict interner,
                                          u64 length) {
    LayoutListElement *element =
        layout_list_lookup_padding(&interner->padding, length);
    if (element != NULL) { return element->layout; }

    Layout *layout = allocate(sizeof(Layout));
    layout_create_padding(layout, length);
    layout_list_append(&interner->padding, NULL, layout);
    return layout;
}

Layout const *
layout_interner_layout_of_primary(LayoutInterner *restrict interner,
                                  TypePrimary primary) {
    switch (primary.kind) {
    case TYPE_PRIMARY_KIND_NIL:  return &interner->nil;
    case TYPE_PRIMARY_KIND_BOOL: return &interner->bool_;
    case TYPE_PRIMARY_KIND_U8:   return &interner->u8_;
    case TYPE_PRIMARY_KIND_U16:  return &interner->u16_;
    case TYPE_PRIMARY_KIND_U32:  return &interner->u32_;
    case TYPE_PRIMARY_KIND_U64:  return &interner->u64_;
    case TYPE_PRIMARY_KIND_I8:   return &interner->i8_;
    case TYPE_PRIMARY_KIND_I16:  return &interner->i16_;
    case TYPE_PRIMARY_KIND_I32:  return &interner->i32_;
    case TYPE_PRIMARY_KIND_I64:  return &interner->i64_;
    default:                     EXP_UNREACHABLE();
    }
}

Layout const *
layout_interner_layout_of_composite(LayoutInterner *restrict interner,
                                    Type const *restrict type) {
    switch (type->kind) {
    case TYPE_COMPOSITE_KIND_TUPLE: {
        LayoutListElement *element = layout_list_lookup(&interner->tuple, type);
        if (element != NULL) { return element->layout; }

        Layout *layout = allocate(sizeof(Layout));
        layout_create_tuple(layout, &type->composite.data.tuple, interner);
    }

    case TYPE_COMPOSITE_KIND_FUNCTION: {
        return &interner->function;
    }

    default: EXP_UNREACHABLE();
    }
}

Layout const *layout_interner_layout_of(LayoutInterner *restrict interner,
                                        Type const *type) {
    switch (type->kind) {
    case TYPE_KIND_PRIMARY:
        return layout_interner_layout_of_primary(interner, type->primary);
    case TYPE_KIND_COMPOSITE:
        return layout_interner_layout_of_composite(interner, type);

    default: EXP_UNREACHABLE();
    }
}
