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

#include "codegen/x86/imr/body/tuple.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void x86_tuple_create(x86_Tuple *restrict tuple) {
    exp_assert(tuple != NULL);
    tuple->length   = 0;
    tuple->capacity = 0;
    tuple->buffer   = NULL;
}

void x86_tuple_destroy(x86_Tuple *restrict tuple) {
    exp_assert(tuple != NULL);
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
    exp_assert(tuple != NULL);
    if (x86_tuple_full(tuple)) { x86_tuple_grow(tuple); }

    tuple->buffer[tuple->length++] = operand;
}
