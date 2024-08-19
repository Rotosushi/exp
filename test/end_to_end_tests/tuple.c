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

int tuple([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int result = EXIT_SUCCESS;

  StringView source_path = SV(EXP_TEST_DIR "/tuple.exp");

  result |=
      test_exp(source_path, "fn main() { const x = (2, 4); return x.1; }", 4);

  result |= test_exp(
      source_path, "fn main() { const x = (2, 4); return x.0 * x.1; }", 8);

  result |= test_exp(source_path,
                     "fn f(x: (i64, i64)) { return x.0 + x.1; } fn main() { "
                     "const x = (1, 2); return f(x); }",
                     3);

  result |= test_exp(source_path,
                     "fn f(x: i64, y: i64) { return (x, y); } fn main() { "
                     "const x = f(4, 2); return x.1 + x.0; }",
                     6);

  return result;
}
