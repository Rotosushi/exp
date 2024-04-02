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

#include "core/compile.h"
#include "env/context.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/process.h"

static void write_file(char const *restrict path, char const *data) {
  FILE *file = file_open(path, "w");

  file_write(data, file);

  file_close(file);
}

static bool compile_test(char const *restrict body) {
  bool failed = 0;
  static char const source[] = EXP_TEST_DIR "/test.exp";
  static char const assembly[] = EXP_TEST_DIR "/test.s";
  static char const object[] = EXP_TEST_DIR "/test.o";

  ContextOptions options = context_options_create();
  path_assign(&options.output, assembly);
  path_assign(&options.source, source);
  Context context = context_create(&options);

  write_file(source, body);

  failed |= (compile(&context) != EXIT_SUCCESS);

  char const *args[] = {assembly, "-o", object, NULL};
  if (process("as", args) != 0) {
    failed |= 1;
  } else {
    file_remove(object);
  }

  file_remove(assembly);
  file_remove(source);
  context_destroy(&context);
  return failed;
}

int compile_tests([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  bool failed = 0;

  failed |= compile_test("const x = 3;");

  if (failed)
    return EXIT_FAILURE;
  else
    return EXIT_SUCCESS;
}