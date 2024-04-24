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

#include "env/context_options.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/panic.h"

ContextOptions context_options_create() {
  ContextOptions options;
  options.source = string_create();
  options.output = string_create();
  return options;
}

ContextOptions
context_options_from_cli_options(CLIOptions *restrict cli_options) {
  ContextOptions options = context_options_create();
  string_assign(&options.source, cli_options->source.buffer);
  string_assign(&options.output, cli_options->output.buffer);
  return options;
}

void context_options_destroy(ContextOptions *restrict options) {
  string_destroy(&(options->source));
  string_destroy(&(options->output));
}
