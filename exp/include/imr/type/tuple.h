// Copyright (C) 2025 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_IMR_TUPLE_TYPE_H
#define EXP_IMR_TUPLE_TYPE_H

#include "support/string.h"

struct Type;

typedef struct TypeTuple {
    u32                 length;
    u32                 capacity;
    struct Type const **types;
} TypeTuple;

void type_tuple_create(TypeTuple *restrict tuple);
void type_tuple_destroy(TypeTuple *restrict tuple_type);
bool type_tuple_equal(TypeTuple const *A, TypeTuple const *B);
bool type_tuple_index_in_bounds(TypeTuple const *restrict tuple, u32 index);
void type_tuple_append(TypeTuple *restrict tuple_type, struct Type const *type);
struct Type const *type_tuple_at(TypeTuple const *restrict tuple, u32 index);

void print_type_tuple(String *restrict string, TypeTuple const *restrict tuple);

#endif // !EXP_IMR_TUPLE_TYPE_H
