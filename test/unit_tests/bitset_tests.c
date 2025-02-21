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

/**
 * @file unit_tests/bitset_tests.c
 */

#include "utility/bitset.h"
#include "utility/pseudo_random.h"
#include "utility/result.h"

static u8 random_index(XorShiftR128PlusState *xorshiftr128plus_state) {
    u64 x = xorshiftr128plus_generate(xorshiftr128plus_state);
    return (u8)(x % bitset_length());
}

i32 bitset_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
    ExpResult result = EXP_SUCCESS;

    XorShiftR128PlusState xorshiftr128plus_state;
    xorshiftr128plus_initialize(&xorshiftr128plus_state, 550415);

    Bitset set_0 = bitset_create();
    if (!bitset_empty(&set_0)) { result = EXP_FAILURE; }

    u8 index = random_index(&xorshiftr128plus_state);
    bitset_set_bit(&set_0, index);
    if (!bitset_check_bit(&set_0, index)) { result = EXP_FAILURE; }

    bitset_clear_bit(&set_0, index);
    if (bitset_check_bit(&set_0, index)) { result = EXP_FAILURE; }

    bitset_assign_bit(&set_0, index, true);
    if (!bitset_check_bit(&set_0, index)) { result = EXP_FAILURE; }

    bitset_assign_bit(&set_0, index, false);
    if (bitset_check_bit(&set_0, index)) { result = EXP_FAILURE; }

    return result;
}
