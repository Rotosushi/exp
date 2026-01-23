/**
 * Copyright (C) 2025 Cade Weinberg
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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <string.h>

#include "support/assert.h"
#include "support/cli_option_parser.h"
#include "support/unreachable.h"

void option_parser_init(OptionParser *restrict parser,
                        Option const *options,
                        i32 option_count) {
    exp_assert(parser != NULL);
    exp_assert(options != NULL);
    parser->option_index = 1;
    parser->option_count = option_count;
    parser->options      = options;
}

static OptionResult unknown_option(char const *restrict argument) {
    return (OptionResult){.option = '?', .argument = argument};
}

static OptionResult missing_argument() {
    return (OptionResult){.option = ':', .argument = NULL};
}

static OptionResult end_of_arguments() {
    return (OptionResult){.option = -1, .argument = NULL};
}

static OptionResult handle_option_argument(OptionParser *restrict parser,
                                           Option const *restrict option,
                                           i32 argc,
                                           char const *argv[]) {
    switch (option->argument_kind) {
    case OPTION_ARGUMENT_NONE: {
        parser->option_index += 1;
        return (OptionResult){.option = option->short_name, .argument = NULL};
        break;
    }

    case OPTION_ARGUMENT_REQUIRED: {
        if (parser->option_index + 1 >= argc) { return missing_argument(); }
        parser->option_index += 2;
        return (OptionResult){.option   = option->short_name,
                              .argument = argv[parser->option_index - 1]};
        break;
    }

    case OPTION_ARGUMENT_OPTIONAL: {
        if (parser->option_index + 1 >= argc) {
            parser->option_index += 1;
            return (OptionResult){.option   = option->short_name,
                                  .argument = NULL};
        }
        parser->option_index += 2;
        return (OptionResult){.option   = option->short_name,
                              .argument = argv[parser->option_index - 1]};
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static OptionResult parse_short_option(OptionParser *restrict parser,
                                       char const *argument,
                                       i32 argc,
                                       char const *argv[]) {
    for (i32 index = 0; index < parser->option_count; ++index) {
        Option const *option = parser->options + index;
        if (option->short_name == argument[0]) {
            return handle_option_argument(parser, option, argc, argv);
        }
    }
    return unknown_option(argument);
}

static OptionResult parse_long_option(OptionParser *restrict parser,
                                      char const *argument,
                                      u64 length,
                                      i32 argc,
                                      char const *argv[]) {
    for (i32 index = 0; index < parser->option_count; ++index) {
        Option const *option = parser->options + index;
        if (strncmp(option->name, argument, length) == 0) {
            return handle_option_argument(parser, option, argc, argv);
        }
    }
    return unknown_option(argument);
}

OptionResult
parse_option(OptionParser *restrict parser, i32 argc, char const *argv[]) {
    exp_assert(parser != NULL);
    exp_assert(argv != NULL);

    if (parser->option_index >= argc) { return end_of_arguments(); }
    char const *arg = argv[parser->option_index];
    while ((arg[0] != '-') && (parser->option_index < argc)) {
        parser->option_index += 1;
        arg = argv[parser->option_index];
    }

    // we know the arg starts with '-' or '--',
    // so we want to consume the '-' or '--' characters
    if (arg[0] == '-') { arg++; }
    if (arg[0] == '-') { arg++; }

    u64 arg_length = strlen(arg);
    if (arg_length == 1) { return parse_short_option(parser, arg, argc, argv); }
    return parse_long_option(parser, arg, arg_length, argc, argv);
}
