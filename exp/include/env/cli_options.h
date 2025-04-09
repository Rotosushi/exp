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

#include "support/bitset.h"
#include "support/string.h"

/*
 * @note: We currently support stopping the compiler
 * at the assembly stage, and the linking stage.
 * And cleanup refers to removing the intermediate
 * files generated during the compilation process.
 *
 * I would like more granualrity in specifying which
 * files get cleaned up. And I would like a more uniform
 * way of refering to which stage in the compilation process
 * we are going to get to. I think an easy way of getting both
 * is just just speak in terms of build artifacts.
 *
 * source -> IR -> assembly (x86-64) -> object (pic) -> executable (pie) |
 * library (static/dynamic)
 *
 * I would like to add a verbose flag, and a trace flag, for
 * inspecting the compilation process.
 *
 *
 */

typedef enum CLIFlags {
    CLI_PROLIX,
    CLI_TRACE,
    CLI_CREATE_IR_ARTIFACT,
    CLI_CREATE_ASSEMBLY_ARTIFACT,
    CLI_CREATE_OBJECT_ARTIFACT,
    CLI_CREATE_EXECUTABLE_ARTIFACT,
    // CLI_CREATE_LIBRARY_ARTIFACT,
    CLI_CLEANUP_IR_ARTIFACT,
    CLI_CLEANUP_ASSEMBLY_ARTIFACT,
    CLI_CLEANUP_OBJECT_ARTIFACT,
} CLIFlags;

typedef struct CLIOptions {
    Bitset flags;
    String source;
    String output;
} CLIOptions;

CLIOptions cli_options_create();
void       cli_options_destroy(CLIOptions *restrict cli_options);

[[nodiscard]] CLIOptions parse_cli_options(i32 argc, char const *argv[]);

#endif // !EXP_UTILITY_CLI_OPTIONS_H
