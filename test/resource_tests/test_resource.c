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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "adt/string.h"
#include "test_resource.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"
#include "utility/process.h"

static char const *exp_path = EXP_BINARY_DIR "/exp/source/exp";

i32 test_exp(StringView source_path, char const *contents, i32 expected_code) {
    i32 result = EXIT_SUCCESS;

    String exe;
    string_initialize(&exe);
    string_assign(&exe, source_path);
    string_replace_extension(&exe, SV(""));
    char const *exe_path = string_to_cstring(&exe);

    FILE *file = file_open(source_path.ptr, "w");
    file_write(contents, file);
    file_close(file);

    char const *exp_args[] = {exp_path, source_path.ptr, NULL};
    i32 exp_result         = process(exp_path, exp_args);
    result |= exp_result;

    if (result == EXIT_SUCCESS) {
        char const *test_args[] = {exe_path, NULL};
        i32 test_result         = process(exe_path, test_args);
        if (test_result != expected_code) {
            file_write("expected code: ", stderr);
            file_write_i64(expected_code, stderr);
            file_write(" actual code: ", stderr);
            file_write_i64(test_result, stderr);
            file_write("\n", stderr);
            result |= EXIT_FAILURE;
        }

        file_remove(exe_path);
    }

    file_remove(source_path.ptr);
    string_destroy(&exe);
    return result;
}

u8 parse_exit_code(StringView path) {
    u64 cursor = 0;
    for (; cursor < path.length; ++cursor) {
        if (path.ptr[cursor] == '.') {
            ++cursor;
            break;
        }
    }

    if (!isdigit(path.ptr[cursor])) { PANIC("path doesn't contain exit code"); }

    char const *exit_code_begin = path.ptr + cursor;
    u64 exit_code_length        = 0;
    while (isdigit(path.ptr[cursor])) {
        ++cursor;
        ++exit_code_length;
    }

    i64 exit_code = str_to_i64(exit_code_begin, exit_code_length);
    if (!i64_in_range_u8(exit_code)) { PANIC("exit code out of range"); }

    return (u8)exit_code;
}

i32 test_resource(StringView path) {
    String exe_string;
    string_initialize(&exe_string);
    string_assign(&exe_string, path);
    string_replace_extension(&exe_string, SV(""));
    char const *exe_path = string_to_cstring(&exe_string);

    u8 exit_code = parse_exit_code(path);

    char const *exp_args[] = {exp_path, path.ptr, nullptr};
    if (process(exp_path, exp_args) == EXIT_FAILURE) {
        string_destroy(&exe_string);
        return EXIT_FAILURE;
    }

    char const *test_args[] = {exe_path, nullptr};
    i32 test_result         = process(exe_path, test_args);
    if (test_result != exit_code) {
        string_destroy(&exe_string);
        file_write("\ntest failed: test resource: ", stderr);
        file_write(path.ptr, stderr);
        file_write("\nexpected exit code: ", stderr);
        file_write_i64(exit_code, stderr);
        file_write(" actual exit code: ", stderr);
        file_write_i64(test_result, stderr);
        file_write("\n", stderr);
        return EXIT_FAILURE;
    }

    file_remove(exe_path);
    string_destroy(&exe_string);
    return EXIT_SUCCESS;
}