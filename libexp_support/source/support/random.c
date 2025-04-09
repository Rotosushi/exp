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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "support/random.h"
#include "support/assert.h"

void splitmix64_seed(SplitMix64State *restrict state, u64 seed) {
    exp_assert(state != nullptr);
    state->state = seed;
}

u64 splitmix64_next(SplitMix64State *state) {
    exp_assert(state != nullptr);
    u64 result = (state->state += 0x9E3779B97F4A7C15);
    result     = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result     = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

void xorshiftr128plus_seed(XorShiftR128PlusState *state, u64 seed) {
    exp_assert(state != nullptr);
    SplitMix64State splitmix64_state = {.state = seed};
    state->state[0]                  = splitmix64_next(&splitmix64_state);
    state->state[1]                  = splitmix64_next(&splitmix64_state);
}

static bool nonzero_state(XorShiftR128PlusState *state) {
    exp_assert(state != nullptr);
    if (state->state[0] != 0) return true;
    return state->state[1] != 0;
}

/*
 * #NOTE: xorshiftr128plus based on the xorshift family
 *   of non-cryptographic pseudo random number generators
 *  https://en.wikipedia.org/wiki/Xorshift
 */
u64 xorshiftr128plus_next(XorShiftR128PlusState *state) {
    exp_assert(state != nullptr);
    exp_assert(nonzero_state(state));
    u64 x           = state->state[0];
    u64 const y     = state->state[1];
    state->state[0] = y;
    x ^= x << 23;
    x ^= x >> 17;
    x ^= y;
    state->state[1] = x + y;
    return x;
}

