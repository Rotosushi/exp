// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_STRING_VIEW_H
#define EXP_UTILITY_STRING_VIEW_H

#include "utility/int_types.h"

typedef struct StringView {
    u64 length;
    char const *ptr;
} StringView;

StringView string_view_create();
StringView string_view_from_str(char const *string, u64 length);
StringView string_view_from_cstring(char const *cstring);
bool string_view_equality(StringView sv1, StringView sv2);
bool string_view_empty(StringView sv);

#define SV(s) ((StringView){.length = sizeof(s) - 1, .ptr = (s)})

#endif // !EXP_UTILITY_STRING_VIEW_H
