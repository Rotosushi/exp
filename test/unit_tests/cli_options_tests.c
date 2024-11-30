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
#include <stdlib.h>
#include <string.h>

#include "env/cli_options.h"

bool test_options(i32 argc, char const *argv[], StringView sv) {
    CLIOptions cli_options;
    parse_cli_options(&cli_options, argc, argv);

    bool failure = 0;
    if (!string_equality(&cli_options.source, sv)) {
        failure |= 1;
    } else {
        failure |= 0;
    }

    cli_options_terminate(&cli_options);
    return failure;
}

i32 cli_options_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
    bool failure = 0;

    i32 test_argc           = 2;
    char const *test_argv[] = {"options_tests", "hello.txt", NULL};

    failure |= test_options(test_argc, test_argv, SV("hello.txt"));

    if (failure) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
