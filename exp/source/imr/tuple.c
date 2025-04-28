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

#include "imr/tuple.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void tuple_create(Tuple *restrict tuple) {
    exp_assert(tuple != NULL);
    tuple->size     = 0;
    tuple->capacity = 0;
    tuple->elements = NULL;
}

void tuple_destroy(Tuple *restrict tuple) {
    exp_assert(tuple != NULL);
    deallocate(tuple->elements);
    tuple_create(tuple);
}

bool tuple_equal(Tuple *A, Tuple *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A == B) { return true; }
    if (A->size != B->size) { return false; }

    for (u64 i = 0; i < A->size; ++i) {
        if (!operand_equality(A->elements[i], B->elements[i])) { return false; }
    }

    return true;
}

static bool tuple_full(Tuple *restrict tuple) {
    return (tuple->size + 1) >= tuple->capacity;
}

static void tuple_grow(Tuple *restrict tuple) {
    Growth_u32 g = array_growth_u32(tuple->capacity, sizeof(*tuple->elements));
    tuple->elements = reallocate(tuple->elements, g.alloc_size);
    tuple->capacity = g.new_capacity;
}

void tuple_append(Tuple *restrict tuple, Operand element) {
    if (tuple_full(tuple)) { tuple_grow(tuple); }

    tuple->elements[tuple->size++] = element;
}
