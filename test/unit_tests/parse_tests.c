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

#include "frontend/parser.h"
#include "utility/config.h"
#include "utility/io.h"

bool test_parse(StringView body) {
  StringView path = SV(EXP_TEST_DIR "/parse.exp");
  FILE *file      = file_open(path, SV("w"));
  file_write(file, body);
  file_close(file);

  char const *exp    = EXP_BUILD_DIR "/exp/source/exp";
  int argc           = 2;
  char const *argv[] = {exp, path.ptr, nullptr};

  CLIOptions options = parse_cli_options(argc, argv);
  Context context    = context_create(&options);

  bool failure = (parse_source(&context) == EXIT_FAILURE);

  context_destroy(&context);
  file_remove(path);

  if (failure) {
    file_write(stderr, SV("failed to parse "));
    file_write(stderr, body);
  }

  return failure;
}

i32 parse_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
  bool failure = 0;

  failure |= test_parse(SV("fn f() { return 0; }"));
  failure |= test_parse(SV("fn f() { return 3 + 3; }"));
  failure |= test_parse(SV("fn f() { return 3 - 5 * 9; }"));
  failure |=
      test_parse(SV("fn f() { return 12; }\n fn g() { return f() + 12; }"));

  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}
