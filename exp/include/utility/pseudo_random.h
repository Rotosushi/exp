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
 * @file utility/pseudo_random.h
 */

#ifndef EXP_UTILITY_PSEUDO_RANDOM_H
#define EXP_UTILITY_PSEUDO_RANDOM_H

#include "int_types.h"

typedef struct XorShiftR128PlusState {
    u64 state[2];
} XorShiftR128PlusState;

void xorshiftr128plus_initialize(XorShiftR128PlusState *state, u64 seed);
u64 xorshiftr128plus_generate(XorShiftR128PlusState *state);

/**
 * @note length is the length of valid array subscript locations
 */
void populate_array_with_random_alphabetic_characters(u64 seed, char *buffer,
                                                      u64 length);

#endif // EXP_UTILITY_PSEUDO_RANDOM_H
