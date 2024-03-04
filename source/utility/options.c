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

#include "filesystem/io.h"
#include "utility/config.h"
#include "utility/options.h"
#include "utility/panic.h"

// static void print_version(FILE *file) { file_write(EXP_VERSION_STRING, file);
// }

// static void print_help(FILE *file) {}

Options options_create() {
  Options options;
  options.source = path_create();
  return options;
}

void options_destroy(Options *restrict options) {
  path_destroy(&(options->source));
}

#if defined(EXP_HOST_OS_LINUX)
#include <getopt.h>

// Options parse_options(int argc, char const *argv[]) {
//   Options options = options_create();

//   return options;
// }

#else
#error "unsupported host OS"
#endif