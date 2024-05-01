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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "backend/emit_x64_linux_assembly.h"
#include "env/context.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/process.h"

i32 emit_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char const *argv[]) {
  srand((unsigned)time(NULL));
  bool failed = 0;

  static char const source[]   = EXP_TEST_DIR "/asm.exp";
  static char const assembly[] = EXP_TEST_DIR "/asm.s";
  static char const object[]   = EXP_TEST_DIR "/asm.o";

  ContextOptions options = context_options_create();
  path_assign(&options.output, assembly);
  path_assign(&options.source, source);
  Context context = context_create(&options);

  StringView g0 = context_intern(&context, string_view_from_cstring("g0"));
  StringView g1 = context_intern(&context, string_view_from_cstring("g1"));
  StringView g2 = context_intern(&context, string_view_from_cstring("g2"));
  Value v0      = value_create_i64(rand());
  Value v1      = value_create_i64(rand());
  Value v2      = value_create_boolean((bool)(rand() % 2));

  context_insert_global_symbol(&context, g0, &v0);
  context_insert_global_symbol(&context, g1, &v1);
  context_insert_global_symbol(&context, g2, &v2);

  emit_x64_linux_assembly(&context);

  char const *args[] = {assembly, "-o", object, NULL};

  i32 code = process("as", args);
  if (code != 0) {
    failed = 1;
  } else {
    file_remove(object);
  }

  file_remove(assembly);

  context_destroy(&context);
  if (failed) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}