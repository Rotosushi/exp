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

#include "imr/type/tuple.h"
#include "imr/type.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void type_tuple_create(TypeTuple *restrict tuple_type) {
    EXP_ASSERT(tuple_type != NULL);
    tuple_type->capacity = 0;
    tuple_type->length   = 0;
    tuple_type->types    = NULL;
}

void type_tuple_destroy(TypeTuple *restrict tuple_type) {
    EXP_ASSERT(tuple_type != NULL);
    tuple_type->capacity = 0;
    tuple_type->length   = 0;
    deallocate(tuple_type->types);
    tuple_type->types = NULL;
}

bool type_tuple_equal(TypeTuple const *A, TypeTuple const *B) {
    EXP_ASSERT(A != NULL);
    EXP_ASSERT(B != NULL);
    if (A == B) { return 1; }

    if (A->length != B->length) { return 0; }

    for (u64 i = 0; i < A->length; ++i) {
        Type const *t = A->types[i];
        Type const *u = B->types[i];

        if (!type_equality(t, u)) { return 0; }
    }

    return 1;
}

bool type_tuple_index_in_bounds(TypeTuple const *restrict tuple, u32 index) {
    EXP_ASSERT(tuple != NULL);
    return tuple->length > index;
}

static bool tuple_type_full(TypeTuple *restrict tuple_type) {
    return (tuple_type->length + 1) >= tuple_type->capacity;
}

static void tuple_type_grow(TypeTuple *restrict tuple_type) {
    Growth_u32 g      = array_growth_u32(tuple_type->capacity, sizeof(Type *));
    tuple_type->types = reallocate(tuple_type->types, g.alloc_size);
    tuple_type->capacity = g.new_capacity;
}

void type_tuple_append(TypeTuple *restrict tuple_type, Type const *type) {
    EXP_ASSERT(tuple_type != NULL);
    EXP_ASSERT(type != NULL);

    if (tuple_type_full(tuple_type)) { tuple_type_grow(tuple_type); }

    tuple_type->types[tuple_type->length] = type;
    tuple_type->length += 1;
}

Type const *type_tuple_at(TypeTuple const *restrict tuple, u32 index) {
    EXP_ASSERT(tuple != NULL);
    EXP_ASSERT(type_tuple_index_in_bounds(tuple, index));
    return tuple->types[index];
}

void print_type_tuple(String *restrict string,
                      TypeTuple const *restrict tuple_type) {
    EXP_ASSERT(string != NULL);
    EXP_ASSERT(tuple_type != NULL);

    string_append(string, SV("("));
    for (u64 i = 0; i < tuple_type->length; ++i) {
        print_type(string, tuple_type->types[i]);

        if (i < (tuple_type->length - 1)) { string_append(string, SV(", ")); }
    }
    string_append(string, SV(")"));
}
