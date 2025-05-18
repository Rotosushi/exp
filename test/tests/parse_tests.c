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

#include "scanning/parser.h"
#include "support/io.h"
#include "support/string.h"
#include "test_resources.h"

static i32 test_parse(StringView path) {
    ContextOptions options = {};
    Context        context;
    context_create(&context, &options);

    Parser parser;
    parser_create(&parser, &context);

    String buffer;
    string_initialize(&buffer);
    file_read_all(&buffer, path);

    parser_setup(&parser, string_to_view(&buffer));

    i32 result = 0;
    while (!parser_done(&parser)) {
        Function expression;
        function_create(&expression);

        if (!parser_parse_expression(&parser, &expression)) {
            SourceLocation location;
            parser_current_source_location(&parser, &location);
            context_print_error(&context, location.file, location.line);
            function_destroy(&expression);
            result += 1;
            break;
        }
    }

    context_destroy(&context);
    return result;
}

i32 parse_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
    i32           result = 0;
    TestResources test_resources;
    test_resources_initialize(&test_resources);

    for (u64 index = 0; index < test_resources.count; ++index) {
        String *resource = test_resources.buffer + index;
        file_write(SV("\ntesting resource: "), stderr);
        file_write(string_to_view(resource), stderr);
        test_parse(string_to_view(resource));
    }

    test_resources_terminate(&test_resources);
    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
