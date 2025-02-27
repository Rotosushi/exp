// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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
