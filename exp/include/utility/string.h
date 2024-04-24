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
#ifndef EXP_UTILITY_STRING_H
#define EXP_UTILITY_STRING_H
#include <stdio.h>

#include "utility/string_view.h"

/**
 * @brief represents a string at runtime.
 *
 */
typedef struct String {
  u64 length;
  u64 capacity;
  char *buffer;
} String;

/*
  the so called "small string optimization" is
  a string structure which looks something like:
  struct string {
    u64 length;
    u64 capacity;
    bool is_small;
    union {
      char *buffer;
      char small[sizeof(char *)];
    };
  };
  so the string is only 1 bool larger than
  the current string structure.
  a fancy implementation could store the
  bool within the allocation of another
  data member, or so I have heard.

  addendum, isn't it true that whenever (length < sizeof(char *))
  the string will be stored in the small string buffer?
  thus, the bool is completely equivalent to the above check
  and is actually unnecessary.
  well, why not make this a #TODO just for fun. (and experience)
*/

/**
 * @brief create a new string
 *
 * @return string
 */
String string_create();

/**
 * @brief free the given string
 *
 * @note frees the strings allocated buffer, if any
 *
 * @param str
 */
void string_destroy(String *restrict str);

/**
 * @brief return a StringView of the entire string
 *
 * @param str
 * @return StringView
 */
StringView string_to_view(String const *restrict str);

/**
 * @brief return a StringView of a subsection of <string>,
 *  starting at <offset> and continuing for <length>.
 *
 * @warning assert((offset <= string->length) && ((offset + length) <=
 * string->length))
 *
 * @param string
 * @param offset
 * @param length
 * @return StringView
 */
StringView string_to_view_at(String const *restrict string, u64 offset,
                             u64 length);

String string_from_view(StringView sv);

String string_from_cstring(char const *restrict cs);

String string_from_file(FILE *restrict file);

/**
 * @brief returns if the string is empty.
 *
 * @param string
 * @return true
 * @return false
 */
bool string_empty(String const *restrict string);

/**
 * @brief compare two strings lexicographically
 *
 * @param s1
 * @param s2
 * @return i32
 */
i32 string_compare(String const *restrict s1, String const *restrict s2);

/**
 * @brief resize the string to be able to hold at least
 * <capacity> characters.
 *
 * @param str the string to resize
 * @param capacity the new capacity of the string
 */
void string_resize(String *restrict str, u64 capacity);

/**
 * @brief resize the string to be able to hold at least
 * <capacity> more characters.
 *
 * @param str
 * @param capacity
 */
void string_reserve_more(String *restrict str, u64 capacity);

/**
 * @brief assigns the string to hold exactly the contents of <data>
 *
 * @param str
 * @param data
 * @param data_length
 */
void string_assign(String *restrict str, const char *restrict data);

void string_assign_view(String *restrict str, StringView sv);

void string_move(String *restrict s1, String *restrict s2);

/**
 * @brief appends <data> to the current contents of <str>
 *
 * @param str
 * @param data
 * @param data_length
 */
void string_append(String *restrict str, const char *restrict data);

void string_append_view(String *restrict str, StringView sv);

/**
 * @brief concatenates <s2> onto the end of <s1>
 *
 * @param s1
 * @param s2
 */
void string_append_string(String *restrict s1, const String *restrict s2);

/**
 * @brief appends <c> onto the end of <str>
 *
 * @param str
 * @param c
 */
void string_append_char(String *restrict str, const char c);

/**
 * @brief erases the substring of str->buffer[offset]
 * to str->buffer[offset + length]
 *
 * @warning assert((offset <= string->length) && ((offset + length) <=
 * string->length))
 *
 * @param str
 * @param offset
 * @param length
 */
void string_erase(String *restrict str, u64 offset, u64 length);

/**
 * @brief insert data into <str>; growing <str> as necessary
 * @param str
 * @param offset
 * @param data
 */
void string_insert(String *restrict str, u64 offset, char const *restrict data);

void print_string(String *restrict s, FILE *restrict file);

#endif // !EXP_UTILITY_STRING_H