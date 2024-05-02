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

#include "core/assemble.h"
#include "core/codegen.h"
#include "core/compile.h"
#include "core/link.h"
#include "core/typecheck.h"
#include "env/cli_options.h"
#include "env/context.h"
#include "frontend/parser.h"

static i32 compile_context(Context *restrict c) {
  if (parse_source(c) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (typecheck(c) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (codegen(c) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

i32 compile(i32 argc, char const *argv[]) {
  CLIOptions cli_options = parse_cli_options(argc, argv);
  Context context        = context_create(&cli_options);

  i32 result = compile_context(&context);

  if ((result != EXIT_FAILURE) && context_do_assemble(&context)) {
    result |= assemble(&context);
  }

  if ((result != EXIT_FAILURE) && context_do_link(&context)) {
    result |= link(&context);
  }

  if (context_do_cleanup(&context)) {
    StringView asm_path = context_assembly_path(&context);
    file_remove(asm_path.ptr);

    StringView obj_path = context_object_path(&context);
    file_remove(obj_path.ptr);
  }

  context_destroy(&context);
  cli_options_destroy(&cli_options);
  return result;
}