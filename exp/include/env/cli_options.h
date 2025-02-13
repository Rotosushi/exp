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

#include "utility/bitset.h"
#include "utility/string.h"

typedef enum CLIFlag {
    CLI_EMIT_IR_ASSEMBLY,
    CLI_EMIT_X86_64_ASSEMBLY,

    CLI_CREATE_ELF_OBJECT,
    CLI_CREATE_ELF_EXECUTABLE,

    CLI_CLEANUP_X86_64_ASSEMBLY,
    CLI_CLEANUP_ELF_OBJECT,
} CLIFlag;

typedef struct CLIOptions {
    Bitset flags;
    String source;
    String output;
} CLIOptions;

void cli_options_initialize(CLIOptions *cli_options);
void cli_options_terminate(CLIOptions *cli_options);

void parse_cli_options(CLIOptions *cli_options, i32 argc, char const *argv[]);

#endif // !EXP_UTILITY_CLI_OPTIONS_H
