/**
 * Copyright (C) 2024 cade
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

// #include "core/compile.h"
#include "env/context.h"
#include "utility/cli_options.h"

i32 main(i32 argc, char const *argv[], [[maybe_unused]] char *envv[]) {
  [[maybe_unused]] CLIOptions cli_options = parse_cli_options(argc, argv);

  // ContextOptions options = context_options_create();
  // options.source         = path_clone(&cli_options.source);
  // options.output         = path_clone(&cli_options.output);

  // Context context = context_create(&options);

  // i32 result = compile(&context);

  // context_destroy(&context);
  // cli_options_destroy(&cli_options);
  // return result;
  return EXIT_SUCCESS;
}