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
#include <time.h>

#include "env/constants.h"
#include "support/random.h"

i32 test_constant(Constants *restrict values, Value *value) {
    Operand operand = constants_append(values, value);

    return value_equal(operand.data.constant, value) ? 0 : 1;
}

i32 constants_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
    XorShiftR128PlusState rng;
    xorshiftr128plus_seed(&rng, (u64)time(NULL));
    Constants constants;
    constants_create(&constants);
    i32 result = 0;

    // #TODO:
    result += test_constant(
        &constants, value_allocate_i64((i64)xorshiftr128plus_next(&rng)));
    result += test_constant(
        &constants, value_allocate_i64((i64)xorshiftr128plus_next(&rng)));
    result += test_constant(
        &constants, value_allocate_i64((i64)xorshiftr128plus_next(&rng)));
    result += test_constant(
        &constants, value_allocate_i64((i64)xorshiftr128plus_next(&rng)));

    constants_destroy(&constants);
    return result;
}
