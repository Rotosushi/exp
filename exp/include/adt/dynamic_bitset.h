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
#ifndef EXP_ADT_DYNAMIC_BITSET_H
#define EXP_ADT_DYNAMIC_BITSET_H

#include "support/scalar.h"

typedef struct DynamicBitset {
    u64  capacity;
    u64 *buffer;
} DynamicBitset;

void dynamic_bitset_create(DynamicBitset *restrict bitset);
void dynamic_bitset_destroy(DynamicBitset *restrict bitset);

void dynamic_bitset_resize(DynamicBitset *restrict bitset, u64 capacity);

void dynamic_bitset_set(DynamicBitset *restrict bitset, u64 index);
void dynamic_bitset_clear(DynamicBitset *restrict bitset, u64 index);
bool dynamic_bitset_check(DynamicBitset *restrict bitset, u64 index);

// #NOTE: more possible useful operations
// union, intersection, subset, strict subset, superset, strict superset,
// complement, difference, cardinality, equality, power set, cartesian-product

#endif // !EXP_ADT_DYNAMIC_BITSET_H
