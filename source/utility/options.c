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

#include "filesystem/io.h"
#include "utility/config.h"
#include "utility/options.h"
#include "utility/panic.h"

static void print_version(FILE *file) {
  file_write(EXP_VERSION_STRING, file);
  file_write("\n", file);
}

static void print_help(FILE *file) {
  file_write("exp [options] <source-file>\n\n", file);
  file_write("  -h print help\n", file);
  file_write("  -v print version\n", file);
  file_write("  -o <filename> set output filename\n", file);
  file_write("\n", file);
}

Options options_create() {
  Options options;
  options.source = path_create();
  options.output = path_create();
  return options;
}

void options_destroy(Options *restrict options) {
  path_destroy(&(options->source));
  path_destroy(&(options->output));
}

#if defined(EXP_HOST_OS_LINUX)
#include <getopt.h>

Options parse_options(int argc, char const *argv[]) {
  Options options = options_create();
  static char const *short_options = "hvo:";

  int option = 0;
  while ((option = getopt(argc, (char *const *)argv, short_options)) != -1) {
    switch (option) {
    case 'h': {
      print_help(stdout);
      exit(EXIT_SUCCESS);
      break;
    }

    case 'v': {
      print_version(stdout);
      exit(EXIT_SUCCESS);
      break;
    }

    case 'o': {
      path_assign(&(options.output), optarg, strlen(optarg));
      break;
    }

    default: {
      char buf[] = {(char)option, '\0'};
      file_write("unknown option [", stderr);
      file_write(buf, stderr);
      file_write("]\n", stderr);
      break;
    }
    }
  }

  if (optind < argc) {
    char const *source = argv[optind];
    path_assign(&(options.source), source, strlen(source));
  } else { // no input file given
    file_write("an input file must be specified.\n", stderr);
    exit(EXIT_SUCCESS);
  }

  // use the input filename as the default
  // base of the output filename
  if (path_empty(&(options.output))) {
    options.output = path_clone(&options.source);
  }

  return options;
}

#else
#error "unsupported host OS"
#endif