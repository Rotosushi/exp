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

#include "utility/string_view.h"

/**
 * @brief holds a set of strings, such that the memory
 * allocated for these strings is all managed in one location,
 * this has the additional benefiet of allowing string
 * comparison outside the string interner to require a single
 * ptr comparison
 */
typedef struct StringInterner {
    u64 capacity;
    u64 count;
    StringView *buffer;
} StringInterner;

void string_interner_initialize(StringInterner *string_interner);
void string_interner_terminate(StringInterner *string_interner);
StringView string_interner_insert(StringInterner *string_interner,
                                  StringView sv);

#endif // !EXP_ENV_STRING_INTERNER_H
