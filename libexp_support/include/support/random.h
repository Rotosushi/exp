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

#ifndef EXP_SUPPORT_RANDOM_H
#define EXP_SUPPORT_RANDOM_H

#include "scalar.h"

typedef struct SplitMix64State {
    u64 state;
} SplitMix64State;

void splitmix64_seed(SplitMix64State *restrict state, u64 seed);
u64 splitmix64_next(SplitMix64State *restrict state);

typedef struct XorShiftR128PlusState {
    u64 state[2];
} XorShiftR128PlusState;

void xorshiftr128plus_seed(XorShiftR128PlusState *restrict state, u64 seed);
u64 xorshiftr128plus_next(XorShiftR128PlusState *restrict state);


#endif // !EXP_SUPPORT_RANDOM_H
