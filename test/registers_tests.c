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

#include "imr/registers.h"

i32 registers_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
  srand((unsigned)time(NULL));
  Registers registers;
  bool failure = 0;

  Value *r0   = registers_at(&registers, 0);
  *r0         = value_create_integer(rand());
  Value *r0_1 = registers_at(&registers, 0);
  failure |= !value_equality(r0, r0_1);

  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}