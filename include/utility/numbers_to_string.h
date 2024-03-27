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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "utility/string.h"

// #TODO: size_t float_safe_strlen(float value);
// #TODO: size_t double_safe_strlen(double value);
// #TODO  char * float_to_string(float value, char *buffer, size_t buf_len);
// #TODO: char * double_to_string(double value, char *buffer, size_t buf_len);

typedef enum Radix {
  RADIX_BINARY = 2,
  RADIX_OCTAL = 8,
  RADIX_DECIMAL = 10,
  RADIX_HEXADECIMAL = 16,
} Radix;

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value when converted
 * into the given radix and the minus sign if value is negative.
 *
 * @note does not include the null terminator.
 * @note the resulting string length does not include the
 * c-style prefixes 0x, 0b, 0
 *
 * @param value the value whose digits are to be stored
 * @param radix the radix to convert the number into
 * @return size_t the length of the string
 */
size_t intmax_safe_strlen(intmax_t value, Radix radix);

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value when converted
 * into the given radix
 *
 * @note does not include the null terminator.
 * @note the resulting string length does not include the
 * c-style prefixes 0x, 0b, 0
 *
 * @param value the value whose digits are to be stored
 * @param radix the radix to convert the number into
 * @return size_t the length of the string
 */
size_t uintmax_safe_strlen(uintmax_t value, Radix radix);

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
char *intmax_to_str(intmax_t value, char *restrict buffer, Radix radix);

/**
 * @brief create a string holding the result of intmax_to_str
 *
 * @note it is the callers responsibility to call string_destroy()
 * on the returned string
 *
 * @param value the value to convert
 * @param radix the radix of the resulting string
 * @return string the returned string
 */
String intmax_to_string(intmax_t value, Radix radix);

void print_intmax(intmax_t value, Radix radix, FILE *file);

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
char *uintmax_to_str(uintmax_t value, char *restrict buffer, Radix radix);

/**
 * @brief create a string holding the result of uintmax_to_str
 *
 * @note it is the callers responsibility to call string_destroy()
 * on the returned string
 *
 * @param value the value to convert
 * @param radix the radix of the resulting string
 * @return string the returned string
 */
String uintmax_to_string(uintmax_t value, Radix radix);

void print_uintmax(uintmax_t value, Radix radix, FILE *file);

#endif // !EXP_UTILITY_NUMBERS_TO_STRING_H