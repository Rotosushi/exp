/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "utility/string_view.h"

/**
 * @brief represents a string at runtime.
 *
 */
typedef struct string {
  size_t length;
  size_t capacity;
  char *buffer;
} string;

/**
 * @brief initialize the given string
 *
 * @warning doesn't free the strings allocated buffer
 * if it exists.
 *
 * @param str
 */
void string_init(string *str);

/**
 * @brief reset the given string
 *
 * @note frees the strings allocated buffer, if any
 *
 * @param str
 */
void string_reset(string *str);

/**
 * @brief return a string_view of the string
 *
 * @param str
 * @return string_view
 */
string_view string_to_view(string const *str);

/**
 * @brief resize the string to be able to hold at least
 * <capacity> characters.
 *
 * @note if <capacity> is less than or equal to the strings
 * current capacity, the strings allocated buffer is not
 * reallocated.
 *
 * @param str the string to resize
 * @param capacity the new capacity of the string
 */
void string_resize(string *str, size_t capacity);

/**
 * @brief resize the string to be able to hold at least
 * <capacity> more characters.
 *
 * @param str
 * @param capacity
 */
void string_reserve_more(string *str, size_t capacity);

/**
 * @brief assigns the string to hold exactly the contents of <data>
 *
 * @param str
 * @param data
 * @param data_length
 */
void string_assign(string *str, const char *data, size_t data_length);

/**
 * @brief appends <data> to the current contents of <str>
 *
 * @param str
 * @param data
 * @param data_length
 */
void string_append(string *str, const char *data, size_t data_length);

/**
 * @brief concatenates <s2> onto the end of <s1>
 *
 * @param s1
 * @param s2
 */
void string_concat(string *s1, string const *s2);