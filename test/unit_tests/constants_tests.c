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
#include <stdlib.h>
#include <time.h>

#include "env/constants.h"

bool test_constant(Constants *values, Value value) {
    Operand index   = constants_append(values, value);
    Value *constant = constants_at(values, index.data.constant);

    if (value_equality(constant, &value)) {
        return 0;
    } else {
        return 1;
    }
}

i32 constants_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
    srand((unsigned)time(NULL));
    Constants constants;
    constants_initialize(&constants);
    bool failure = 0;

    // #TODO:
    failure |= test_constant(&constants, value_create_i64(rand()));
    failure |= test_constant(&constants, value_create_i64(rand()));
    failure |= test_constant(&constants, value_create_i64(rand()));
    failure |= test_constant(&constants, value_create_i64(rand()));

    constants_terminate(&constants);
    if (failure) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
