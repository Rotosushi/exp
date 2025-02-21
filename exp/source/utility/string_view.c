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
 * @file utility/string_view.c
 */

#include "utility/string_view.h"
#include "utility/assert.h"
#include "utility/memory.h"

StringView string_view_create() {
    StringView sv;
    sv.length = 0;
    sv.ptr    = "";
    return sv;
}

StringView string_view_from_str(char const *string, u64 length) {
    EXP_ASSERT(string != nullptr);
    StringView sv = {length, string};
    return sv;
}

static u64 cstring_length(char const *cstring) {
    EXP_ASSERT(cstring != nullptr);
    u64 length = 0;
    while (*cstring != '\0') {
        ++cstring;
        ++length;
    }
    return length;
}

StringView string_view_from_cstring(char const *cstring) {
    EXP_ASSERT(cstring != nullptr);
    StringView sv = {cstring_length(cstring), cstring};
    return sv;
}

bool string_view_equality(StringView sv1, StringView sv2) {
    if (sv1.ptr == sv2.ptr) { return true; }
    if (sv1.length != sv2.length) { return false; }
    return (memory_compare(sv1.ptr, sv1.length, sv2.ptr, sv2.length) == 0);
}

bool string_view_empty(StringView sv) { return sv.length == 0; }

/*
void print_string_view(StringView sv, FILE *restrict file) {
  for (u64 i = 0; i < sv.length; ++i) {
    if (fputc(sv.ptr[i], file) == EOF) { PANIC_ERRNO("fputc failed"); }
  }
}
*/
