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
#include <stdlib.h>

#include "backend/register_set.h"

int register_set_tests([[maybe_unused]] int argc,
                       [[maybe_unused]] char **argv) {
  bool failure = false;
  RegisterSet set;

  register_set_preallocate(&set, REG_RAX);
  Register r0 = register_set_next_available(&set);
  failure |= (r0 != REG_RBX);

  register_set_release(&set, REG_RAX);
  Register r1 = register_set_next_available(&set);
  failure |= (r1 != REG_RAX);

  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}