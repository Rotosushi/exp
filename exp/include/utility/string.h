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
// #include <stdbool.h>
// #include <stdio.h>

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

void string_initialize(String *string);
void string_terminate(String *str);
StringView string_to_view(String const *str);
char const *string_to_cstring(String const *str);
char *string_data(String *str);
void string_from_view(String *string, StringView view);
void string_from_cstring(String *string, char const *cstring);
bool string_empty(String const *string);
bool string_equality(String const *str, StringView sv);
void string_resize(String *str, u64 capacity);
void string_assign(String *str, StringView sv);
void string_assign_string(String *dst, String const *src);
void string_append(String *str, StringView sv);
void string_append_string(String *dst, String const *src);
void string_append_i64(String *str, i64 i);
void string_append_u64(String *str, u64 u);

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
void string_erase(String *str, u64 offset, u64 length);

/**
 * @brief insert data into <str>; growing <str> as necessary
 * @param str
 * @param offset
 * @param data
 */
void string_insert(String *str, u64 offset, StringView sv);

StringView string_extension(String const *string);

/**
 * @brief treats the string as a unix filepath, and replaces the extension
 *   present already with the one given. if there is no extension present
 *   then this adds the given extension, if the given extension is empty
 *   then this removes the extension.
 */
void string_replace_extension(String *str, StringView ext);

#endif // !EXP_UTILITY_STRING_H
