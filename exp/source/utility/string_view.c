/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
