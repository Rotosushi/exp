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

/**
 * @file utility/pseudo_random.c
 */

#include "utility/pseudo_random.h"
#include "utility/assert.h"

typedef struct SplitMix64State {
    u64 state;
} SplitMix64State;

static u64 splitmix64_generate(SplitMix64State *state) {
    EXP_ASSERT(state != nullptr);
    u64 result = (state->state += 0x9E3779B97F4A7C15);
    result     = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result     = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

void xorshiftr128plus_initialize(XorShiftR128PlusState *state, u64 seed) {
    EXP_ASSERT(state != nullptr);
    SplitMix64State splitmix64_state = {.state = seed};
    state->state[0]                  = splitmix64_generate(&splitmix64_state);
    state->state[1]                  = splitmix64_generate(&splitmix64_state);
}

static bool nonzero_state(XorShiftR128PlusState *state) {
    EXP_ASSERT(state != nullptr);
    if (state->state[0] != 0) return true;
    return state->state[1] != 0;
}

/*
 * #NOTE: xorshiftr128plus based on the xorshift family
 *   of non-cryptographic pseudo random number generators
 *  https://en.wikipedia.org/wiki/Xorshift
 */
u64 xorshiftr128plus_generate(XorShiftR128PlusState *state) {
    EXP_ASSERT(state != nullptr);
    EXP_ASSERT(nonzero_state(state));
    u64 x           = state->state[0];
    u64 const y     = state->state[1];
    state->state[0] = y;
    x ^= x << 23;
    x ^= x >> 17;
    x ^= y;
    state->state[1] = x + y;
    return x;
}

static bool is_alphabetic(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static char random_alphabet_character(XorShiftR128PlusState *state) {
    EXP_ASSERT(state != nullptr);
    char x = xorshiftr128plus_generate(state) % i8_MAX;
    while (!is_alphabetic(x)) {
        x = xorshiftr128plus_generate(state) & i8_MAX;
    }
    return x;
}

void populate_array_with_random_alphabetic_characters(u64 seed, char *buffer,
                                                      u64 length) {
    EXP_ASSERT(buffer != nullptr);
    XorShiftR128PlusState state;
    xorshiftr128plus_initialize(&state, seed);
    for (u64 index = 0; index < (length - 1); ++index) {
        buffer[index] = random_alphabet_character(&state);
    }
    buffer[length] = '\0';
}
