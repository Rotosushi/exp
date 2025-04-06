// Copyright (C) 2024 Cade Weinberg
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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_STRING_INTERNER_H
#define EXP_ENV_STRING_INTERNER_H

#include "support/string_view.h"

/**
 * @brief We use simple strings instead of general strings
 * because the general strings are small string optimized
 * and that means we cannot safely return a view into the
 * small string and freely grow the interned strings.
 * as reallocating the buffer of strings may move the interned
 * string, and any small strings will have their data stored directly
 * in the previous buffer, thus invalidating the string view returned by the
 * string interner. Causing chaos somewhere down the line.
 */
typedef struct SimpleString {
    u64  length;
    char data[];
} SimpleString;

/**
 * @brief holds a set of strings, such that the memory
 * allocated for these strings is all managed in one location,
 * this has the additional benefiet of allowing string
 * comparison outside the string interner to require a single
 * memory comparison.
 */
typedef struct StringInterner {
    u64            capacity;
    u64            count;
    SimpleString **buffer;
} StringInterner;

StringInterner string_interner_create();

void string_interner_destroy(StringInterner *restrict string_interner);

StringView string_interner_insert(StringInterner *restrict string_interner,
                                  StringView sv);

#endif // !EXP_ENV_STRING_INTERNER_H
