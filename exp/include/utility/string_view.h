// Copyright (C) 2025 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file utility/string_view.h
 */

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
