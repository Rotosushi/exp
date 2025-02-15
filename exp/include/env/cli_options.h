// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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
