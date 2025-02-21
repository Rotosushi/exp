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

/**
 * @file unit_tests/string_interner_tests.c
 */

// #include <stdlib.h>

#include "env/string_interner.h"
#include "utility/result.h"

i32 string_interner_tests([[maybe_unused]] i32 argc,
                          [[maybe_unused]] char *argv[]) {
    StringInterner si;
    string_interner_initialize(&si);
    bool failure = 0;

    StringView sv0 = (string_interner_insert(&si, SV("hello")));
    StringView sv1 = (string_interner_insert(&si, SV("world")));
    failure |= string_view_equality(sv0, sv1);

    StringView sv2 = (string_interner_insert(&si, SV("hello")));
    failure |= !string_view_equality(sv0, sv2);
    failure |= string_view_equality(sv1, sv2);

    string_interner_terminate(&si);
    if (failure) {
        return EXP_FAILURE;
    } else {
        return EXP_SUCCESS;
    }
}
