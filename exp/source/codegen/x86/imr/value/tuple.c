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

#include "codegen/x86/imr/value/tuple.h"
#include "codegen/GAS/directives.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void x86_tuple_create(x86_Tuple *restrict tuple) {
    EXP_ASSERT(tuple != NULL);
    tuple->length   = 0;
    tuple->capacity = 0;
    tuple->buffer   = NULL;
}

void x86_tuple_destroy(x86_Tuple *restrict tuple) {
    EXP_ASSERT(tuple != NULL);
    deallocate(tuple->buffer);
    x86_tuple_create(tuple);
}

static bool x86_tuple_full(x86_Tuple const *restrict tuple) {
    return (tuple->length + 1) >= tuple->capacity;
}

static void x86_tuple_grow(x86_Tuple *restrict tuple) {
    Growth_u32 g    = array_growth_u32(tuple->capacity, sizeof(x86_Operand));
    tuple->buffer   = reallocate(tuple->buffer, g.alloc_size);
    tuple->capacity = g.new_capacity;
}

void x86_tuple_append(x86_Tuple *restrict tuple, x86_Operand operand) {
    EXP_ASSERT(tuple != NULL);
    if (x86_tuple_full(tuple)) { x86_tuple_grow(tuple); }

    tuple->buffer[tuple->length++] = operand;
}

x86_Operand *x86_tuple_at(x86_Tuple *restrict tuple, u32 index) {
    EXP_ASSERT(tuple != NULL);
    EXP_ASSERT(index <= tuple->length);
    return tuple->buffer + index;
}

bool x86_tuple_equal(x86_Tuple const *A, x86_Tuple const *B) {
    EXP_ASSERT(A != NULL);
    EXP_ASSERT(B != NULL);
    if (A == B) { return true; }
    if (A->length != B->length) { return false; }

    for (u64 i = 0; i < A->length; ++i) {
        if (!x86_operand_equal(A->buffer[i], B->buffer[i])) { return false; }
    }

    return true;
}

void print_x86_tuple(String *restrict buffer,
                     x86_Tuple const *restrict tuple,
                     Type const *type) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(tuple != NULL);
    EXP_ASSERT(type != NULL);

    Layout const *layout = type->layout;
    // It is safe to access this as if it was a tuple layout,
    // unless I made a silly mistake, or some memory was corrupted.
    EXP_ASSERT_DEBUG(layout->kind == LAYOUT_KIND_TUPLE);
    LayoutTuple const *tuple_layout = &layout->data.tuple;
    // So, the length of the tuple and it's layout should be the "same"
    // except that the layout takes into account the padding that must be
    // present between elements. and these take up element slots.
    // Thus the layout length is the same or greater than.
    for (u32 tuple_index = 0, layout_index = 0; tuple_index < tuple->length;
         ++tuple_index, ++layout_index) {
        x86_Operand element = tuple->buffer[tuple_index];
        // #NOTE: we need to be careful here, if we create a global tuple
        // which contains an address as an element, the behavior as a global
        // initializer will not work, and will likely be a bug caught in the
        // assembler.
        print_x86_operand(buffer, element);
        if (tuple_index >= (tuple->length - 1)) { continue; }

        EXP_ASSERT(layout_index < (tuple_layout->length - 1));
        Layout const *next_layout = tuple_layout->buffer[layout_index + 1];
        if (next_layout->kind != LAYOUT_KIND_PADDING) { continue; }

        gas_directive_zero(next_layout->data.padding, buffer);
        ++layout_index;
        // Since we "know" that the layout length is the same or less,
        // this assertion should never fire.
        EXP_ASSERT_DEBUG(layout_index < tuple_layout->length);
    }
}
