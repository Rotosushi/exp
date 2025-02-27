// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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
