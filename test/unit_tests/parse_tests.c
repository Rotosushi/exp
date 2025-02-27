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

#include "frontend/parser.h"
#include "utility/io.h"

bool test_parse(StringView body) {
    Context context;
    context_initialize(
        &context, bitset_create(), string_view_create(), string_view_create());

    bool failure =
        (parse_buffer(body.ptr, body.length, &context) == EXIT_FAILURE);

    context_terminate(&context);

    if (failure) {
        file_write(body, program_error);
        file_write(SV(" failed to parse."), program_error);
    }

    return failure;
}

i32 parse_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
    bool failure = 0;

    failure |= test_parse(SV("fn f() { return 0; }"));
    failure |= test_parse(SV("fn f() { return 3 + 3; }"));
    failure |= test_parse(SV("fn f() { return 3 - 5 * 9; }"));
    failure |=
        test_parse(SV("fn f() { return 12; }\n fn g() { return f() + 12; }"));

    if (failure) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
