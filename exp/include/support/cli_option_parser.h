// Copyright (C) 2025 Cade Weinberg
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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file cli_option_parser.h
 * @brief Command line option parser
 */

#ifndef EXP_SUPPORT_CLI_OPTION_PARSER_H
#define EXP_SUPPORT_CLI_OPTION_PARSER_H

#include "support/scalar.h"

typedef enum OptionArgumentKind {
    OPTION_ARGUMENT_NONE,
    OPTION_ARGUMENT_REQUIRED,
    OPTION_ARGUMENT_OPTIONAL,
} OptionArgumentKind;

typedef struct Option {
    char const *name;
    char const *description;
    u8 argument_kind;
    char short_name;
} Option;

typedef struct OptionParser {
    i32 option_index;
    i32 option_count;
    Option const *options;
} OptionParser;

typedef struct OptionResult {
    i32 option;
    char const *argument;
} OptionResult;

void option_parser_init(OptionParser *restrict parser,
                        Option const *options,
                        i32 option_count);

OptionResult
parse_option(OptionParser *restrict parser, i32 argc, char const *argv[]);

#endif // !EXP_SUPPORT_CLI_OPTION_PARSER_H
