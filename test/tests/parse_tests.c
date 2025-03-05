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

#include "scanning/parser.h"
#include "test_resources.h"

static Context init_context() {
    CLIOptions options = cli_options_create();
    Context result     = context_create(&options);
    return result;
}

i32 test_parse(StringView contents) {
    Context context = init_context();

    i32 result = parse_buffer(contents.ptr, contents.length, &context);

    context_destroy(&context);

    if (result != EXIT_SUCCESS) {
        file_write(SV(" failed to parse:\n"), stderr);
        file_write(contents, stderr);
        file_write(SV("\n"), stderr);
    }

    return result;
}

i32 parse_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
    i32 result = 0;
    TestResources test_resources;
    test_resources_initialize(&test_resources);

    for (u64 index = 0; index < test_resources.count; ++index) {
        String *resource = test_resources.buffer + index;
        file_write(SV("\ntesting resource: "), stderr);
        file_write(string_to_view(resource), stderr);

        FILE *file      = file_open(string_to_cstring(resource), "r");
        String contents = string_from_file(file);
        file_close(file);
        result += test_parse(string_to_view(&contents));
        string_destroy(&contents);
    }

    test_resources_terminate(&test_resources);
    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
