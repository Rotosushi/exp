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

/**
 * @file test/resource_tests/main.c
 */

#include "test_resource.h"
#include "test_resources.h"
#include "utility/io.h"
#include "utility/result.h"

i32 main() {
    i32 result = EXP_SUCCESS;
    TestResources test_resources;
    test_resources_initialize(&test_resources);

    for (u64 index = 0; index < test_resources.count; ++index) {
        String *resource = test_resources.buffer + index;
        String buffer;
        string_initialize(&buffer);
        string_append(&buffer, SV("\ntesting resource: "));
        string_append(&buffer, string_to_view(resource));
        file_write(string_to_view(&buffer), program_error);
        string_terminate(&buffer);
        result |= test_resource(string_to_view(resource));
    }

    test_resources_terminate(&test_resources);
    return result;
}
