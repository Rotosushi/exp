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

bool test_constants(Constants *restrict constants, Value value) {
  Operand index   = constants_add(constants, value);
  Value *constant = constants_at(constants, index.index);

  if (value_equality(constant, &value)) {
    return 0;
  } else {
    return 1;
  }
}

i32 constants_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
  srand((unsigned)time(NULL));
  Constants constants = constants_create();
  bool failure        = 0;

  // #TODO:
  failure |= test_constants(&constants, value_create_i64(rand()));
  failure |= test_constants(&constants, value_create_i64(rand()));
  failure |= test_constants(&constants, value_create_i64(rand()));
  failure |= test_constants(&constants, value_create_i64(rand()));

  // failure |= test_constants(
  //     &constants, value_create_string_literal(
  //                     string_view_from_string("hello", sizeof("hello"))));
  // failure |= test_constants(
  //     &constants, value_create_string_literal(
  //                     string_view_from_string("world", sizeof("world"))));
  // failure |= test_constants(&constants,
  //                           value_create_string_literal(string_view_from_string(
  //                               "hello, world", sizeof("hello, world"))));

  constants_destroy(&constants);
  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}
