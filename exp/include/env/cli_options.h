// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_UTILITY_CLI_OPTIONS_H
#define EXP_UTILITY_CLI_OPTIONS_H

#include "env/context_options.h"
#include "support/string.h"

typedef struct CLIOptions {
    ContextOptions context_options;
    String         source;
} CLIOptions;

void cli_options_create(CLIOptions *restrict cli_options);
void cli_options_destroy(CLIOptions *restrict cli_options);

void parse_cli_options(i32         argc,
                       char const *argv[],
                       CLIOptions *restrict cli_options);

#endif // !EXP_UTILITY_CLI_OPTIONS_H
