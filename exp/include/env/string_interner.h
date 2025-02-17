// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

/**
 * @file utility/string_interner.h
 */

#ifndef EXP_ENV_STRING_INTERNER_H
#define EXP_ENV_STRING_INTERNER_H

#include "utility/constant_string.h"

/**
 * @brief holds a set of strings, such that the memory
 * allocated for these strings is all managed in one location,
 * this has the additional benefiet of allowing string
 * comparison outside the string interner to require a single
 * ptr comparison
 */
typedef struct StringInterner {
    u32 capacity;
    u32 count;
    ConstantString **buffer;
} StringInterner;

void string_interner_initialize(StringInterner *string_interner);
void string_interner_terminate(StringInterner *string_interner);
StringView string_interner_insert(StringInterner *string_interner,
                                  StringView sv);

#endif // !EXP_ENV_STRING_INTERNER_H
