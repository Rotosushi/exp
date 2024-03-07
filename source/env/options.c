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

#include "env/options.h"
#include "filesystem/io.h"
#include "utility/config.h"
#include "utility/panic.h"

Options options_create() {
  Options options;
  options.source = path_create();
  options.output = path_create();
  return options;
}

Options options_clone(Options *restrict other) {
  Options options = options_create();
  options.source = path_clone(&other->source);
  options.output = path_clone(&other->output);
  return options;
}

Options options_from_cli_options(CLIOptions *restrict cli_options) {
  Options options = options_create();
  options.source = path_clone(&cli_options->source);
  options.output = path_clone(&cli_options->output);
  return options;
}

void options_destroy(Options *restrict options) {
  path_destroy(&(options->source));
  path_destroy(&(options->output));
}
