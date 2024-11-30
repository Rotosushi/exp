/**
 * Copyright (C) 2024 Cade Weinberg
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

#ifndef EXP_UTILITY_BITSET_H
#define EXP_UTILITY_BITSET_H

#include "utility/int_types.h"

/**
 * @brief a fixed size bitset with capacity of 64 set members.
 */
typedef struct Bitset {
    u64 bits;
} Bitset;

Bitset bitset_create();

void bitset_set_bit(Bitset *bitset, u8 bit_position);
void bitset_clear_bit(Bitset *bitset, u8 bit_position);
bool bitset_check_bit(Bitset *bitset, u8 bit_position);

// #TODO:
//  Bitset bitset_complement(Bitset A);
//  Bitset bitset_union(Bitset A, Bitset B);
//  Bitset bitset_subset(Bitset A, Bitset B);
//  Bitset bitset_intersection(Bitset A, Bitset B);
//  Bitset bitset_difference(Bitset A, Bitset B);
//  Bitset bitset_disjoint(Bitset A, Bitset B);

#endif // EXP_UTILITY_BITSET_H
