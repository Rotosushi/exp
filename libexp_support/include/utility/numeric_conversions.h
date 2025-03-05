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
#ifndef EXP_UTILITY_NUMBERS_TO_STRING_H
#define EXP_UTILITY_NUMBERS_TO_STRING_H

#include "utility/int_types.h"

// TODO:
//    u64 f32_safe_strlen(float value);
//    u64 f64_safe_strlen(double value);
//    char * f32_to_str(float value, char *buffer);
//    char * f64_to_str(double value, char *buffer);
// of note is that the assembler does not accept floating point
// literals, and instead reads integer literals and stores them
// as floats. effectively the integer literal is just there to
// specify the bit pattern of the float value. so the above functions
// are not useful in the backend.

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value when converted
 * into base 10. plus the minus sign if <value> is negative
 */
u64 i64_safe_strlen(i64 value);

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value when converted
 * into base 10
 */
u64 u64_safe_strlen(u64 value);

/**
 * @brief write the digits of <value> into <buffer>
 * converted into the given radix
 */
char *i64_to_str(i64 value, char *restrict buffer);

char *u64_to_str(u64 value, char *restrict buffer);

bool str_to_i64(i64 *result, char const *restrict buffer, u64 length);

bool str_to_u64(u64 *result, char const *restrict buffer, u64 length);

#endif // !EXP_UTILITY_NUMBERS_TO_STRING_H
