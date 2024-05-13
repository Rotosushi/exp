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
#include <stdio.h>
#include <stdlib.h>

#include "utility/config.h"
#include "utility/io.h"

static char const *source_path = EXP_TEST_DIR "/test.exp";
static char const *exe_path    = EXP_TEST_DIR "/test";
static char const *compile_command =
    EXP_BUILD_DIR "/exp/source/exp " EXP_TEST_DIR "/test.exp";
static char const *test_command = EXP_TEST_DIR "/test";

static int test_exp(char const *contents, int expected_code) {
  int result = EXIT_SUCCESS;

  FILE *file = file_open(source_path, "w");
  file_write(contents, file);
  file_close(file);

  int exp_status = system(compile_command);
  if (!WIFEXITED(exp_status)) {
    result |= EXIT_FAILURE;
  } else if (WEXITSTATUS(exp_status) != 0) {
    result |= EXIT_FAILURE;
  }

  if (result == EXIT_SUCCESS) {
    int test_status = system(test_command);
    if (WIFEXITED(test_status)) {
      result |= (WEXITSTATUS(test_status) == expected_code) ? EXIT_SUCCESS
                                                            : EXIT_FAILURE;
    } else {
      result |= EXIT_FAILURE;
    }

    file_remove(exe_path);
  }

  file_remove(source_path);

  return result;
}

int test_trivial_constant_folding() {
  int result = EXIT_SUCCESS;

  result |= test_exp("fn main() { return 3 + 3; }", 6);
  result |= test_exp("fn main() { return 3 - 3; }", 0);
  result |= test_exp("fn main() { return 3 * 3; }", 9);
  result |= test_exp("fn main() { return 3 / 3; }", 1);
  result |= test_exp("fn main() { return 3 % 3; }", 0);

  return result;
}

int test_addition() {
  int result = EXIT_SUCCESS;

  result |=
      test_exp("fn main() { const x = 3; const y = 3; return x + y; }", 6);
  result |= test_exp("fn main() { const x = 3; return x + 3; }", 6);
  result |= test_exp("fn main() { const x = 3; return 3 + x; }", 6);

  return result;
}

int test_subtraction() {
  int result = EXIT_SUCCESS;

  result |=
      test_exp("fn main() { const x = 3; const y = 3; return x - y; }", 0);
  result |= test_exp("fn main() { const x = 3; return x - 3; }", 0);
  result |= test_exp("fn main() { const x = 3; return 3 - x; }", 0);

  return result;
}

int test_multiplication() {
  int result = EXIT_SUCCESS;

  result |=
      test_exp("fn main() { const x = 3; const y = 3; return x * y; }", 9);
  result |= test_exp("fn main() { const x = 3; return x * 3; }", 9);
  result |= test_exp("fn main() { const x = 3; return 3 * x; }", 9);

  return result;
}

int test_division() {
  int result = EXIT_SUCCESS;

  result |=
      test_exp("fn main() { const x = 9; const y = 3; return x / y; }", 3);
  result |= test_exp("fn main() { const x = 9; return x / 3; }", 3);
  result |= test_exp("fn main() { const x = 3; return 9 / x; }", 3);

  return result;
}

int test_modulus() {
  int result = EXIT_SUCCESS;

  result |=
      test_exp("fn main() { const x = 9; const y = 3; return x % y; }", 0);
  result |= test_exp("fn main() { const x = 9; return x % 3; }", 0);
  result |= test_exp("fn main() { const x = 3; return 9 % x; }", 0);

  return result;
}

int end_to_end_tests([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int result = EXIT_SUCCESS;

  result |= test_exp("fn main() { return 0; }", 0);

  result |= test_trivial_constant_folding();
  result |= test_addition();
  result |= test_subtraction();
  result |= test_multiplication();
  result |= test_division();
  result |= test_modulus();

  return result;
}