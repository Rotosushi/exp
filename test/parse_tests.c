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

#include "frontend/parser.h"

static Context init_context() {
  ContextOptions options = context_options_create();
  Context result         = context_create(&options);
  return result;
}

bool test_parse(char const *body) {
  Context context = init_context();

  bool failure = (parse(body, &context) == EXIT_FAILURE);

  context_destroy(&context);

  if (failure) {
    fputs(body, stderr);
    fputs(" failed to parse.", stderr);
  }

  return failure;
}

i32 parse_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
  bool failure = 0;

  failure |= test_parse("const x = 3;");
  failure |= test_parse("const x = true;");
  failure |= test_parse("const x = false;");
  failure |= test_parse("const x = nil;");
  // failure |= test_parse("const x = \"Hello, World\";");

  failure |= test_parse("const x = -42;");
  failure |= test_parse("const x = 3 + 3;");
  failure |= test_parse("const x = 3 - 2;");
  failure |= test_parse("const x = 3 * 3;");
  failure |= test_parse("const x = 3 / 3;");
  failure |= test_parse("const x = 3 % 3;");

  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}