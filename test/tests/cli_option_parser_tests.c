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
#include <stdlib.h>
#include <string.h>

#include "support/cli_option_parser.h"
#include "support/scalar.h"

static Option const options[] = {
    {    .name          = "help",
     .description   = "print help",
     .argument_kind = OPTION_ARGUMENT_NONE,
     .short_name    = 'h'},
    { .name          = "version",
     .description   = "print version",
     .argument_kind = OPTION_ARGUMENT_NONE,
     .short_name    = 'v'},
    {  .name          = "output",
     .description   = "set output filename",
     .argument_kind = OPTION_ARGUMENT_REQUIRED,
     .short_name    = 'o'},
    { .name          = "compile",
     .description   = "emit an object file",
     .argument_kind = OPTION_ARGUMENT_NONE,
     .short_name    = 'c'},
    {.name          = "assemble",
     .description   = "emit an assembly file",
     .argument_kind = OPTION_ARGUMENT_NONE,
     .short_name    = 's'},
};

static i32 option_count = sizeof(options) / sizeof(Option);

i32 test_single_option_no_argument(i32 argc,
                                   char const *argv[],
                                   Option const *restrict expected) {
    OptionParser parser;
    option_parser_init(&parser, options, option_count);

    OptionResult parsed = parse_option(&parser, argc, argv);

    i32 result = (parsed.option == expected->short_name) ? 0 : 1;
    if (expected->argument_kind == OPTION_ARGUMENT_REQUIRED) {
        result += (parsed.argument != NULL) ? 0 : 1;
    }
    return result;
}

i32 test_single_option_required_argument(i32 argc,
                                         char const *argv[],
                                         Option const *restrict expected,
                                         char const *restrict expected_argument,
                                         u64 expected_argument_length) {
    OptionParser parser;
    option_parser_init(&parser, options, option_count);

    OptionResult parsed = parse_option(&parser, argc, argv);

    i32 result = (parsed.option == expected->short_name) ? 0 : 1;
    result +=
        (strncmp(parsed.argument, expected_argument, expected_argument_length))
            ? 0
            : 1;
    return result;
}

i32 cli_option_parser_tests([[maybe_unused]] i32 argc,
                            [[maybe_unused]] char **argv) {
    i32 result = 0;

    {
        i32 test_argc           = 2;
        char const *test_argv[] = {"cli_option_parser_tests", "-h", NULL};
        Option const *expected  = options + 0;
        result +=
            test_single_option_no_argument(test_argc, test_argv, expected);
    }

    {
        i32 test_argc           = 2;
        char const *test_argv[] = {"cli_option_parser_tests", "-v", NULL};
        Option const *expected  = options + 1;
        result +=
            test_single_option_no_argument(test_argc, test_argv, expected);
    }

    {
        i32 test_argc           = 3;
        char const *test_argv[] = {
            "cli_option_parser_tests", "-o", "output.txt", NULL};
        Option const *expected = options + 2;
        result += test_single_option_required_argument(
            test_argc, test_argv, expected, "output.txt", 10);
    }

    {
        i32 test_argc           = 2;
        char const *test_argv[] = {"cli_option_parser_tests", "-c", NULL};
        Option const *expected  = options + 3;
        result +=
            test_single_option_no_argument(test_argc, test_argv, expected);
    }

    {
        i32 test_argc           = 2;
        char const *test_argv[] = {"cli_option_parser_tests", "-s", NULL};
        Option const *expected  = options + 4;
        result +=
            test_single_option_no_argument(test_argc, test_argv, expected);
    }

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
