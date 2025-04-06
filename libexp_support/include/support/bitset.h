/*
 * Filename: /home/cade/source/exp/libexp_support/include/support/bitset.h
 * Path: /home/cade/source/exp/libexp_support/include/support
 * Created Date: Friday, March 21st 2025, 1:10:19 pm
 * Author: Cade Weinberg
 *
 * Copyright (c) 2025 Cade Weinberg
 */

// Copyright (C) 2024 Cade Weinberg
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

#ifndef EXP_SUPPORT_BITSET_H
#define EXP_SUPPORT_BITSET_H

#include "support/scalar.h"

typedef struct Bitset {
    u64 data;
} Bitset;

/**
 * @brief Create a new Bitset
 *
 * @return Bitset the new Bitset
 */
inline Bitset bitset_create() { return (Bitset){0}; }

/**
 * @brief Set the bit at the given index
 *
 * @param bitset the Bitset to set the bit in
 * @param index the index of the bit to set
 */
inline void bitset_set(Bitset *bitset, u8 index) {
    bitset->data |= (1ULL << index);
}

/**
 * @brief Clear the bit at the given index
 *
 * @param bitset the Bitset to clear the bit in
 * @param index the index of the bit to clear
 */
inline void bitset_clear(Bitset *bitset, u8 index) {
    bitset->data &= ~(1ULL << index);
}

/**
 * @brief Check if the bit at the given index is set
 *
 * @param bitset the Bitset to check
 * @param index the index of the bit to check
 * @return true if the bit is set
 * @return false if the bit is not set
 */
inline bool bitset_check(Bitset const *bitset, u8 index) {
    return ((bitset->data >> index) & 1ULL);
}

#endif // !EXP_SUPPORT_BITSET_H
