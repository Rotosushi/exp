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
 * @file utility/constant_string.h
 */

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
