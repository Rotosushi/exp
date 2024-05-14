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
  union {
    char *ptr;
    char buffer[sizeof(char *)];
  };
} String;

String string_create();

void string_destroy(String *restrict str);

StringView string_to_view(String const *restrict str);

char const *string_to_cstring(String const *restrict str);

String string_from_view(StringView sv);

String string_from_file(FILE *restrict file);

bool string_empty(String const *restrict string);

bool string_eq(String const *restrict str, StringView sv);

void string_resize(String *restrict str, u64 capacity);

void string_assign(String *restrict str, StringView sv);

void string_assign_string(String *restrict dst, String const *restrict src);

void string_append(String *restrict str, StringView sv);

void string_append_string(String *restrict dst, String const *restrict src);

void string_append_i64(String *restrict str, i64 i);

void string_append_u64(String *restrict str, u64 u);

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
void string_insert(String *restrict str, u64 offset, StringView sv);

void string_replace_extension(String *restrict str, StringView ext);

#endif // !EXP_UTILITY_STRING_H