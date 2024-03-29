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

#include "core/compile.h"
#include "env/context.h"
#include "utility/cli_options.h"

int main(int argc, char const *argv[], [[maybe_unused]] char *envv[]) {
  CLIOptions cli_options = parse_options(argc, argv);
  Options options;
  options.source = path_clone(&cli_options.source);
  options.output = path_clone(&cli_options.output);

  Context context = context_create(&options);

  int result = compile(&context);

  context_destroy(&context);
  cli_options_destroy(&cli_options);
  return result;
}