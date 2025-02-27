// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_CONSTANT_STRING_H
#define EXP_UTILITY_CONSTANT_STRING_H

#include "utility/string_view.h"

typedef struct ConstantString {
    u64 length;
    char buffer[];
} ConstantString;

ConstantString *constant_string_allocate(StringView contents);
void constant_string_deallocate(ConstantString *string);

StringView constant_string_to_view(ConstantString *string);
bool constant_string_equality(ConstantString *string, StringView view);

#endif // EXP_UTILITY_CONSTANT_STRING_H
