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
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/io.h"
#include "support/numeric_conversions.h"
#include "support/panic.h"
#include "support/string.h"

String string_create() {
    String str;
    str.length    = 0;
    str.capacity  = sizeof(char *);
    str.buffer[0] = '\0';
    return str;
}

void string_initialize(String *restrict str) {
    assert(str != NULL);
    str->length    = 0;
    str->capacity  = sizeof(char *);
    str->buffer[0] = '\0';
}

static bool string_is_small(String const *restrict str) {
    return str->capacity <= sizeof(char *);
}

void string_destroy(String *restrict str) {
    assert(str != NULL);
    if (!string_is_small(str)) {
        deallocate(str->ptr);
        str->ptr = NULL;
    }
    str->length   = 0;
    str->capacity = sizeof(char *);
}

StringView string_to_view(String const *restrict str) {
    assert(str != NULL);
    StringView sv = string_view_create();
    sv            = string_view_from_str(string_to_cstring(str), str->length);
    return sv;
}

char const *string_to_cstring(String const *restrict str) {
    if (string_is_small(str)) {
        return str->buffer;
    } else {
        return str->ptr;
    }
}

void string_assign(String *restrict str, StringView sv) {
    string_destroy(str);
    if ((sv.length == u64_MAX) || (sv.length + 1 == u64_MAX)) {
        PANIC("cannot allocate more than u64_MAX.");
    }
    str->length   = sv.length;
    str->capacity = max_u64(sv.length + 1, str->capacity);

    if (str->length < sizeof(char *)) {
        memcpy(str->buffer, sv.ptr, str->length);
        str->buffer[str->length] = '\0';
    } else {
        str->ptr = callocate(str->capacity, sizeof(char));
        memcpy(str->ptr, sv.ptr, str->length);
    }
}

void string_assign_string(String *restrict dst, String const *restrict src) {
    string_destroy(dst);
    string_assign(dst, string_to_view(src));
}

String string_from_view(StringView sv) {
    String string = string_create();
    string_assign(&string, sv);
    return string;
}

String string_from_cstring(char const *cstring) {
    return string_from_view(string_view_from_cstring(cstring));
}

String string_from_file(FILE *restrict file) {
    String s    = string_create();
    u64    flen = file_length(file);
    string_resize(&s, flen);
    if (flen < sizeof(char *)) {
        file_read(s.buffer, flen, file);
    } else {
        file_read(s.ptr, flen, file);
    }

    s.length = flen;
    return s;
}

bool string_empty(String const *restrict string) {
    assert(string != NULL);
    return string->length == 0;
}

bool string_eq(String const *restrict str, StringView sv) {
    assert(str != NULL);
    if (str->length != sv.length) { return 0; }

    if (string_is_small(str)) {
        return strncmp(str->buffer, sv.ptr, sv.length) == 0;
    }

    return strncmp(str->ptr, sv.ptr, sv.length) == 0;
}

void string_resize(String *restrict str, u64 capacity) {
    assert(str != NULL);
    if (string_is_small(str)) {
        if (capacity >= sizeof(char *)) {
            char *buf     = callocate(capacity, sizeof(char));
            str->capacity = capacity;
            memcpy(buf, str->buffer, str->length);
            str->ptr = buf;
        } // else the small capacity can hold the given capacity
        return;
    }

    str->ptr              = reallocate(str->ptr, capacity);
    str->capacity         = capacity;
    str->ptr[str->length] = '\0';
}

void string_append(String *restrict str, StringView sv) {
    if (sv.length == 0) { return; }

    if ((str->length + sv.length) >= str->capacity) {
        Growth_u64 g =
            array_growth_u64(str->capacity + sv.length, sizeof(*str->ptr));
        string_resize(str, g.new_capacity);
    }

    if (string_is_small(str)) {
        memcpy(str->buffer + str->length, sv.ptr, sv.length);
        str->length += sv.length;
        str->buffer[str->length] = '\0';
    } else {
        memcpy(str->ptr + str->length, sv.ptr, sv.length);
        str->length += sv.length;
        str->ptr[str->length] = '\0';
    }
}

void string_append_string(String *restrict dst, String const *restrict src) {
    assert(dst != NULL);
    assert(src != NULL);
    string_append(dst, string_to_view(src));
}

void string_append_i64(String *restrict str, i64 i) {
    u64   len = i64_safe_strlen(i);
    char  buf[len + 1];
    char *r = i64_to_str(i, buf);
    if (r == NULL) { PANIC("conversion failed"); }
    buf[len] = '\0';
    string_append(str, string_view_from_str(buf, len));
}

void string_append_u64(String *restrict str, u64 u) {
    u64   len = u64_safe_strlen(u);
    char  buf[len + 1];
    char *r = u64_to_str(u, buf);
    if (r == NULL) { PANIC("conversion failed"); }
    buf[len] = '\0';
    string_append(str, string_view_from_str(buf, len));
}

/*
  essentially can be broken into these cases

  1 offset == 0, length == str->length

  2 offset == 0, length < str->length

  3 offset > 0, (offset + length) == str->length

  4 offset > 0, (offset + length) < str->length
*/
void string_erase(String *restrict str, u64 offset, u64 length) {
    assert(str != NULL);
    assert(offset <= str->length);
    assert((offset + length) <= str->length);

    if ((offset == 0) && (length == str->length)) {
        // erase the entire buffer
        if (string_is_small(str)) {
            str->buffer[0] = '\0';
        } else {
            str->ptr[0] = '\0';
        }
        str->length = 0;
        return;
    }

    // erase <length> characters starting from <str->buffer + offset>
    if (string_is_small(str)) {
        char *pos         = str->buffer + offset;
        char *rest        = pos + length;
        u64   rest_length = (u64)((str->buffer + str->length) - rest);
        memmove(pos, rest, rest_length);
        u64 new_length          = offset + rest_length;
        str->buffer[new_length] = '\0';
        str->length             = new_length;
    } else {
        char *pos         = str->ptr + offset;
        char *rest        = pos + length;
        u64   rest_length = (u64)((str->ptr + str->length) - rest);
        memmove(pos, rest, rest_length);
        u64 new_length       = offset + rest_length;
        str->ptr[new_length] = '\0';
        str->length          = new_length;
    }
}

/*
  there are four cases that need to be considered.

  1 offset == 0, length < str->length

  2 offset == 0, length >= str->length

  3 offset > 0, offset + length < str->length

  4 offset > 0, offset + length >= str->length

  case 1, we can just write the data into the existing buffer
    new_length == existing length
  case 3, we can just write the data into the existing buffer
    new_length == existing length

  case 2, we have to resize the existing buffer, then we can write

  case 4, we have to resize the existing buffer, then we can write
*/
void string_insert(String *restrict str, u64 offset, StringView sv) {
    assert(str != NULL);
    assert(offset <= str->length);

    if ((offset + sv.length) >= str->capacity) {
        string_resize(str, (offset + sv.length) + str->length);
        u64 added_length = (offset + sv.length) - str->length;
        str->length += added_length;
    }

    if (string_is_small(str)) {
        memcpy(str->buffer + offset, sv.ptr, sv.length);
        str->buffer[offset + sv.length] = '\0';
    } else {
        memcpy(str->ptr + offset, sv.ptr, sv.length);
        str->ptr[offset + sv.length] = '\0';
    }
}

void string_replace_extension(String *restrict str, StringView ext) {
    assert(str != NULL);
    // the string is something like
    // /some/kind/of/file.txt
    // or
    // /some/kind/of/.file.txt
    // or
    // /some/kind/of/.file
    // of
    // /some/kind/of/file.with.multiple.extensions

    // search for the last '/' in the string
    u64   length = str->length;
    u64   cursor = length;
    char *buffer = string_is_small(str) ? str->buffer : str->ptr;
    while ((cursor != 0) && (buffer[cursor] != '/')) {
        --cursor;
    }

    // set the cursor to the first char in the filename
    if (buffer[cursor] == '/') { ++cursor; }

    // if the first character in the path is '.' ignore it
    if (buffer[cursor] == '.') { ++cursor; }

    // find the beginning of the final extension
    // or the end of the filename
    u64 extension = cursor;
    while ((cursor < length)) {
        if (buffer[cursor] == '.') { extension = cursor; }
        ++cursor;
    }

    if (ext.length == 0) {
        // remove the extension
        buffer[extension] = '\0';
        str->length       = extension;
    } else {
        if (ext.ptr[0] != '.') {
            if (buffer[extension] != '.') {
                string_append(str, SV("."));
            } else {
                ++extension;
            }
        }
        // insert the given extension.
        string_insert(str, extension, ext);
    }
}

StringView string_extension(String const *restrict str) {
    assert(str != NULL);
    u64         length = str->length;
    u64         cursor = length;
    char const *buffer = string_is_small(str) ? str->buffer : str->ptr;

    // search for the last '.' in the string
    while ((cursor != 0) && (buffer[cursor] != '.')) {
        --cursor;
    }

    return string_view_from_str(buffer + cursor, length - cursor);
}

// void print_string(String *restrict s, FILE *restrict file) {
//   file_write(s->buffer, file);
// }
