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

#include "support/io.h"
#include "test_exp.h"
#include "test_resources.h"

i32 resource_tests([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    i32 result = EXIT_SUCCESS;
    TestResources test_resources;
    test_resources_initialize(&test_resources);

    for (u64 index = 0; index < test_resources.count; ++index) {
        String *resource = test_resources.buffer + index;
        file_write(SV("\ntesting resource: "), stderr);
        file_write(string_to_view(resource), stderr);
        result |= test_source(string_to_view(resource));
    }

    test_resources_terminate(&test_resources);
    return result;
}
