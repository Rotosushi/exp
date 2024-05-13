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
#include <string.h>

#include "adt/string.h"
#include "test_exp.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/numeric_conversions.h"
#include "utility/process.h"

static char const *exp_path = EXP_BUILD_DIR "/exp/source/exp";

i32 test_exp(char const *source_path, char const *contents, i32 expected_code) {
  i32 result = EXIT_SUCCESS;

  String exe = string_create();
  string_assign(&exe, source_path, strlen(source_path));
  string_replace_extension(&exe, "", 0);
  char const *exe_path = string_to_cstring(&exe);

  FILE *file = file_open(source_path, "w");
  file_write(contents, file);
  file_close(file);

  char const *exp_args[] = {exp_path, source_path, NULL};
  i32 exp_result         = process(exp_path, exp_args);
  result |= exp_result;

  if (result == EXIT_SUCCESS) {
    char const *test_args[] = {exe_path, NULL};
    i32 test_result         = process(exe_path, test_args);
    if (test_result != expected_code) {
      file_write("expected code: ", stderr);
      print_i64(expected_code, RADIX_DECIMAL, stderr);
      file_write(" actual code: ", stderr);
      print_i64(test_result, RADIX_DECIMAL, stderr);
      file_write("\n", stderr);
      result |= EXIT_FAILURE;
    }

    file_remove(exe_path);
  }

  file_remove(source_path);
  string_destroy(&exe);
  return result;
}