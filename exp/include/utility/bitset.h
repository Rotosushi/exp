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

/**
 * @file utility/bitset.h
 */

#ifndef EXP_UTILITY_BITSET_H
#define EXP_UTILITY_BITSET_H

#include "utility/int_types.h"

/**
 * @brief a fixed size bitset with capacity set members.
 */
typedef struct Bitset {
    u32 bits;
} Bitset;

#define bitset_length() 32

Bitset bitset_create();

bool bitset_empty(Bitset *bitset);
void bitset_set_bit(Bitset *bitset, u8 bit_position);
void bitset_clear_bit(Bitset *bitset, u8 bit_position);
void bitset_assign_bit(Bitset *bitset, u8 bit_position, bool state);
bool bitset_check_bit(Bitset *bitset, u8 bit_position);

// #TODO:
//  Bitset bitset_complement(Bitset A);
//  Bitset bitset_union(Bitset A, Bitset B);
//  Bitset bitset_subset(Bitset A, Bitset B);
//  Bitset bitset_intersection(Bitset A, Bitset B);
//  Bitset bitset_difference(Bitset A, Bitset B);
//  Bitset bitset_disjoint(Bitset A, Bitset B);

#endif // EXP_UTILITY_BITSET_H
