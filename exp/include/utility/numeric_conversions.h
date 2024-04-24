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
#include "utility/io.h"

// TODO:
//    u64 f32_safe_strlen(float value);
//    u64 f64_safe_strlen(double value);
//    char * f32_to_str(float value, char *buffer, u64 buf_len);
//    char * f64_to_str(double value, char *buffer, u64 buf_len);
// of note is that the assembler does not accept floating point
// literals, and instead reads integer literals and stores them
// as floats. effectively the integer literal is just there to
// specify the bit pattern of the float value. so the above functions
// are not useful in the backend.

typedef enum Radix {
  RADIX_BINARY      = 2,
  RADIX_OCTAL       = 8,
  RADIX_DECIMAL     = 10,
  RADIX_HEXADECIMAL = 16,
} Radix;

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value when converted
 * into the given radix and the minus sign if value is negative.
 *
 * @note does not include the null terminator.
 * @note does not include the c-style prefixes 0x, 0b, 0
 *
 * @param value the value whose digits are to be stored
 * @param radix the radix to convert the number into
 * @return u64 the length of the string
 */
u64 i64_safe_strlen(i64 value, Radix radix);

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value when converted
 * into the given radix
 *
 * @note does not include the null terminator.
 * @note does not include the c-style prefixes 0x, 0b, 0
 *
 * @param value the value whose digits are to be stored
 * @param radix the radix to convert the number into
 * @return u64 the length of the string
 */
u64 u64_safe_strlen(u64 value, Radix radix);

/**
 * @brief write the digits of <value> into <buffer>
 * converted into the given radix
 *
 * @note the buffer must be large enough to hold the given value.
 * @note the radix can be any value between 2 and 32.
 * @note the c style prefixes 0x, 0b, 0, are not prefixed to
 * the resulting string.
 *
 * @param value the number to convert
 * @param buffer the buffer to write the string into
 * @param radix the radix of the resulting string
 * @return char* the end of the string, or NULL if conversion failed.
 */
char *i64_to_str(i64 value, char *restrict buffer, Radix radix);

void print_i64(i64 value, Radix radix, FILE *file);

/**
 * @brief write the digits of <value> into <buffer>
 * converted into the given base
 *
 * @note the buffer must be large enough to hold the given value.
 * @note the base can be any value between 2 and 32.
 * @note the c style prefixes 0x, 0b, 0, are not prefixed to
 * the resulting string.
 *
 * @param value the number to convert
 * @param buffer the buffer to write the string into
 * @param radix the radix of the resulting string
 * @return char* the end of the string, or NULL if conversion failed.
 */
char *u64_to_str(u64 value, char *restrict buffer, Radix radix);

void print_u64(u64 value, Radix radix, FILE *file);

i64 str_to_i64(char const *restrict buffer, u64 length, Radix radix);

u64 str_to_u64(char const *restrict buffer, u64 length, Radix radix);

#endif // !EXP_UTILITY_NUMBERS_TO_STRING_H