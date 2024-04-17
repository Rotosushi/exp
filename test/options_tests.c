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
#include <string.h>

#include "utility/cli_options.h"

static bool s1_same_as_s2(const char *restrict s1, u64 s1_len,
                          const char *restrict s2, u64 s2_len) {
  if (s1_len != s2_len) {
    return 1;
  }

  if (memcmp(s1, s2, s1_len) != 0) {
    return 1;
  }

  return 0;
}

bool test_options(int argc, char const *argv[], char const *d1, u64 d1_len) {
  CLIOptions cli_options = parse_cli_options(argc, argv);

  bool failure = 0;
  if (s1_same_as_s2(cli_options.source.string.buffer,
                    cli_options.source.string.length, d1, d1_len)) {
    failure |= 1;
  } else {
    failure |= 0;
  }

  cli_options_destroy(&cli_options);
  return failure;
}

int options_tests([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  bool failure = 0;

  int test_argc           = 2;
  char const *test_argv[] = {"options_tests", "hello.txt", NULL};

  failure |=
      test_options(test_argc, test_argv, "hello.txt", strlen("hello.txt"));

  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}