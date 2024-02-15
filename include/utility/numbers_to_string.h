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
#pragma once
#include <stddef.h>
#include <stdint.h>

// #TODO: handle the c-style prefixes 0x, 0b, 0, for
// bases 16, 2, and 8, respectively

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value plus null and the
 * minus sign if value is negative.
 *
 * @note the base can be any value between 2 and 32
 * @note the resulting string length does not include the
 * c-style prefixes 0x, 0b, 0 for bases other than 10
 *
 * @param value the value whose digits are to be stored
 * @param base the base (radix) to convert the number into
 * @return size_t the length of the string plus null
 */
size_t intmax_safe_strlen(intmax_t value, unsigned base);

/**
 * @brief return the length of the string which can hold
 * all of the digits in the given value plus null.
 *
 * @note the base can be any value between 2 and 32
 * @note the resulting string length does not include the
 * c-style prefixes 0x, 0b, 0 for bases other than 10
 *
 * @param value the value whose digits are to be stored
 * @param base the base (radix) to convert to number into
 * @return size_t the length of the string plus null
 */
size_t uintmax_safe_strlen(uintmax_t value, unsigned base);

/**
 * @brief write the digits of <value> into <buffer>
 * converted into the given base
 *
 * @note the buffer must be large enough to hold the given value
 * @note the base can be any value between 2 and 32
 * @note the c style prefixes 0x, 0b, 0, are not prefixed to
 * the resulting string.
 *
 * @param value the number to convert
 * @param buffer the buffer to write the string into
 * @param base the base (or radix) of the resulting string
 * @return char* the end of the string, or NULL if conversion failed.
 */
char *intmax_to_str(intmax_t value, char *buffer, unsigned base);

/**
 * @brief write the digits of <value> into <buffer>
 * converted into the given base
 *
 * @note the buffer must be large enough to hold the given value
 * @note the base can be any value between 2 and 32
 * @note the c style prefixes 0x, 0b, 0, are not prefixed to
 * the resulting string.
 *
 * @param value the number to convert
 * @param buffer the buffer to write the string into
 * @param base the base (or radix) of the resulting string
 * @return char* the end of the string, or NULL if conversion failed.
 */
char *uintmax_to_str(uintmax_t value, char *buffer, unsigned base);