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
 * @file utility/constant_string.c
 */

#include "utility/constant_string.h"
#include "utility/alloc.h"
#include "utility/assert.h"
#include "utility/memory.h"

ConstantString *constant_string_allocate(StringView contents) {
    ConstantString *string =
        callocate(1, sizeof(ConstantString) + contents.length + 1);
    EXP_ASSERT(string != nullptr);
    string->length = contents.length;
    memory_copy(string->buffer, string->length, contents.ptr, contents.length);
    return string;
}

void constant_string_deallocate(ConstantString *string) {
    EXP_ASSERT(string != nullptr);
    deallocate(string);
}

StringView constant_string_to_view(ConstantString *string) {
    EXP_ASSERT(string != nullptr);
    return string_view_from_str(string->buffer, string->length);
}

bool constant_string_equality(ConstantString *string, StringView view) {
    EXP_ASSERT(string != nullptr);
    if (string->length != view.length) return false;
    return memory_compare(string->buffer, string->length, view.ptr,
                          view.length) == 0;
}
