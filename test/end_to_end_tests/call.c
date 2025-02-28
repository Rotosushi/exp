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

  result |= test_exp(source_path,
                     "fn f(a: i64) { return a; } fn main() { return f(12); }",
                     12);

  result |= test_exp(
      source_path,
      "fn f(a: i64) { return a; } fn main() { return f(6) + f(3) + f(3); }",
      12);

  result |= test_exp(
      source_path,
      "fn f(a: i64, b: i64) { return a + b; } fn main() { return f(12, 24);}",
      36);

  result |= test_exp(
      source_path,
      "fn f(a: i64, b: i64, c: i64, d: i64, e: i64, f: i64, g: i64) { return a "
      "+ b + c + d + e + f + g; } fn main() { return f(1, 2, 3, 4, 5, 6, 7);}",
      28);

  result |= test_exp(source_path,
                     "fn f(a: i64, b: i64, c: i64, d: i64, e: i64, f: i64, g: "
                     "i64, h: i64, i: i64, j: i64) {"
                     "return a + b + c + d + e + f + g + h + i + j;}"
                     "fn main() { return f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10); }",
                     55);

  return result;
}
