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
#include "env/context_options.h"

#define CHK_BIT(U, b) (((U) >> b) & 1)

ContextOptions context_options_create(CLIOptions *restrict cli_options) {
  ContextOptions options = {.flags = cli_options->flags};

  if (!string_empty(&cli_options->source)) {
    string_assign(&options.source, cli_options->source.buffer);

    string_assign(&options.assembly, cli_options->source.buffer);
    string_replace_extension(&options.assembly, ".s");

    string_assign(&options.object, cli_options->source.buffer);
    string_replace_extension(&options.object, ".o");
  }

  if (!string_empty(&cli_options->output)) {
    string_assign(&options.output, cli_options->output.buffer);
  }

  return options;
}

void context_options_destroy(ContextOptions *restrict options) {
  string_destroy(&options->source);
  string_destroy(&options->assembly);
  string_destroy(&options->object);
  string_destroy(&options->output);
}

bool context_options_do_assemble(ContextOptions *restrict options) {
  return CHK_BIT(options->flags, CLI_DO_ASSEMBLE);
}

bool context_options_do_link(ContextOptions *restrict options) {
  return CHK_BIT(options->flags, CLI_DO_LINK);
}

bool context_options_do_cleanup(ContextOptions *restrict options) {
  return CHK_BIT(options->flags, CLI_DO_CLEANUP);
}

#undef CHK_BIT
