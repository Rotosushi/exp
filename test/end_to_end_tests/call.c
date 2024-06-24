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

#include "test_exp.h"
#include "utility/config.h"

int call([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int result = EXIT_SUCCESS;

  StringView source_path = SV(EXP_TEST_DIR "/call.exp");

  result |= test_exp(
      source_path, "fn f() { return 0; } fn main() { return f(); }", 0);
  result |= test_exp(
      source_path, "fn f() { return 255; } fn main() { return f(); }", 255);
  result |= test_exp(
      source_path, "fn f() { return 25; } fn main() { return f() + f(); }", 50);

  return result;
}