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

int comment([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int result = EXIT_SUCCESS;

  StringView source_path = SV(EXP_TEST_DIR "/comment.exp");

  result |= test_exp(
      source_path, "/* This is a comment */fn main() { return 0; }", 0);

  result |= test_exp(source_path,
                     "/* this is a */ /* comment split between two blocks */ "
                     "fn main() { return 1; }",
                     1);
  result |= test_exp(source_path,
                     "/* this comment is */ fn main() { return 2; } /* "
                     "interspersed with code */",
                     2);

  result |= test_exp(
      source_path,
      "/* this is a /* nested comment */ block */ fn main() { return 3; }",
      3);

  result |= test_exp(source_path,
                     "/* this comment */ fn main() { /* is inside, /* nested,"
                     "/* nested, */ */ */ return 4; } /* and after the code*/",
                     4);

  return result;
}
