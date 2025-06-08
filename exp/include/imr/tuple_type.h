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

typedef struct TupleType {
    u32                 length;
    u32                 capacity;
    struct Type const **types;
} TupleType;

void tuple_type_create(TupleType *restrict tuple);
void tuple_type_destroy(TupleType *restrict tuple_type);
bool tuple_type_equal(TupleType const *A, TupleType const *B);
bool tuple_type_index_in_bounds(TupleType const *restrict tuple, u32 index);
void tuple_type_append(TupleType *restrict tuple_type, struct Type const *type);
struct Type const *tuple_type_at(TupleType const *restrict tuple, u32 index);

void print_tuple_type(String *restrict string, TupleType const *restrict tuple);

#endif // !EXP_IMR_TUPLE_TYPE_H
