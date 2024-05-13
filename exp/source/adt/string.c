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

#include "adt/string.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/minmax.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

String string_create() {
  String str;
  str.length    = 0;
  str.capacity  = sizeof(char *);
  str.buffer[0] = '\0';
  return str;
}

static bool string_small(String const *restrict str) {
  return str->capacity == sizeof(char *);
}

void string_destroy(String *restrict str) {
  assert(str != NULL);
  if (!string_small(str)) {
    free(str->ptr);
    str->ptr = NULL;
  }
  str->length   = 0;
  str->capacity = sizeof(char *);
}

StringView string_to_view(String const *restrict str) {
  assert(str != NULL);
  StringView sv = string_view_create();
  sv            = string_view_from_string(string_to_cstring(str), str->length);
  return sv;
}

char const *string_to_cstring(String const *restrict str) {
  if (string_small(str)) {
    return str->buffer;
  } else {
    return str->ptr;
  }
}

void string_assign(String *restrict str, char const *restrict data,
                   u64 length) {
  string_destroy(str);
  if ((length == u64_MAX) || (length + 1 == u64_MAX)) {
    PANIC("cannot allocate more than u64_MAX.");
  }
  str->length   = length;
  str->capacity = ulmax(length + 1, str->capacity);

  if (str->length < sizeof(char *)) {
    memcpy(str->buffer, data, str->length);
    str->buffer[str->length] = '\0';
  } else {
    str->ptr = callocate(str->capacity, sizeof(char));
    memcpy(str->ptr, data, str->length);
  }
}

void string_assign_sv(String *restrict str, StringView sv) {
  string_assign(str, sv.ptr, sv.length);
}

void string_assign_string(String *restrict dst, String const *restrict src) {
  string_destroy(dst);
  if (string_small(src)) {
    string_assign(dst, src->buffer, src->length);
  } else {
    string_assign(dst, src->ptr, src->length);
  }
}

String string_from_view(StringView sv) {
  String string = string_create();
  string_assign(&string, sv.ptr, sv.length);
  return string;
}

String string_from_file(FILE *restrict file) {
  String s = string_create();
  u64 flen = file_length(file);
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

bool string_eq(String const *restrict str, char const *restrict data,
               u64 length) {
  assert(str != NULL);
  if (str->length != length) {
    return 0;
  }

  if (string_small(str)) {
    return strncmp(str->buffer, data, length) == 0;
  }

  return strncmp(str->ptr, data, length) == 0;
}

void string_resize(String *restrict str, u64 capacity) {
  assert(str != NULL);
  if (string_small(str)) {
    if (capacity >= sizeof(char *)) {
      Growth g      = array_growth_u64(capacity, sizeof(char));
      char *buf     = callocate(g.new_capacity, sizeof(char));
      str->capacity = g.new_capacity;
      memcpy(buf, str->buffer, str->length);
      str->ptr = buf;
    }
    return;
  }

  Growth g              = array_growth_u64(capacity, sizeof(char));
  str->ptr              = reallocate(str->ptr, g.new_capacity);
  str->capacity         = g.new_capacity;
  str->ptr[str->length] = '\0';
}

static void string_append_impl(String *restrict str, char const *restrict data,
                               u64 size) {
  if (size == 0) {
    return;
  }

  if ((str->length + size) >= str->capacity) {
    string_resize(str, str->capacity + size);
  }

  if (string_small(str)) {
    memcpy(str->buffer + str->length, data, size);
    str->length += size;
    str->buffer[str->length] = '\0';
  } else {
    memcpy(str->ptr + str->length, data, size);
    str->length += size;
    str->ptr[str->length] = '\0';
  }
}

void string_append(String *restrict str, const char *restrict data) {
  assert(str != NULL);
  string_append_impl(str, data, strlen(data));
}

void string_append_sv(String *restrict str, StringView sv) {
  assert(str != NULL);
  string_append_impl(str, sv.ptr, sv.length);
}

void string_append_string(String *restrict dst, String const *restrict src) {
  assert(dst != NULL);
  assert(src != NULL);
  if (string_small(src)) {
    string_append_impl(dst, src->buffer, src->length);
  } else {
    string_append_impl(dst, src->ptr, src->length);
  }
}

void string_append_i64(String *restrict str, i64 i) {
  u64 len = i64_safe_strlen(i, RADIX_DECIMAL);
  char buf[len + 1];
  char *r = i64_to_str(i, buf, RADIX_DECIMAL);
  if (r == NULL) {
    PANIC("conversion failed");
  }
  buf[len] = '\0';
  string_append_impl(str, buf, len);
}

void string_append_u64(String *restrict str, u64 u) {
  u64 len = u64_safe_strlen(u, RADIX_DECIMAL);
  char buf[len + 1];
  char *r = u64_to_str(u, buf, RADIX_DECIMAL);
  if (r == NULL) {
    PANIC("conversion failed");
  }
  buf[len] = '\0';
  string_append_impl(str, buf, len);
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
    if (string_small(str)) {
      str->buffer[0] = '\0';
    } else {
      str->ptr[0] = '\0';
    }
    str->length   = 0;
    str->capacity = sizeof(char *);
    return;
  }

  // erase <length> characters starting from <str->buffer + offset>
  if (string_small(str)) {
    char *pos       = str->buffer + offset;
    char *rest      = pos + length;
    u64 rest_length = (u64)((str->buffer + str->length) - rest);
    memmove(pos, rest, rest_length);
    u64 new_length          = offset + rest_length;
    str->buffer[new_length] = '\0';
    str->length             = new_length;
  } else {
    char *pos       = str->ptr + offset;
    char *rest      = pos + length;
    u64 rest_length = (u64)((str->ptr + str->length) - rest);
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
void string_insert(String *restrict str, u64 offset, char const *restrict data,
                   u64 length) {
  assert(str != NULL);
  assert(offset <= str->length);

  if ((offset + length) >= str->capacity) {
    string_resize(str, (offset + length));
    u64 added_length = (offset + length) - str->length;
    str->length += added_length;
  }

  if (string_small(str)) {
    memcpy(str->buffer + offset, data, length);
    str->buffer[offset + length] = '\0';
  } else {
    memcpy(str->ptr + offset, data, length);
    str->ptr[offset + length] = '\0';
  }
}

void string_replace_extension(String *restrict p1, const char *restrict p2,
                              u64 p2_length) {
  assert(p1 != NULL);
  // the string is something like
  // /some/kind/of/file.txt
  // or
  // /some/kind/of/.file.txt
  // or
  // /some/kind/of/.file

  // search for the last '/' in the string
  u64 length   = p1->length;
  u64 cursor   = length;
  char *buffer = string_small(p1) ? p1->buffer : p1->ptr;
  while ((cursor != 0) && (buffer[cursor] != '/')) {
    --cursor;
  }

  // set the cursor to the first char in the filename
  if (buffer[cursor] == '/') {
    ++cursor;
  }

  // if the first character in the path is '.' ignore it
  if (buffer[cursor] == '.') {
    ++cursor;
  }

  // find the beginning of the extension
  // or the end of the filename
  while ((cursor < length) && (buffer[cursor] != '.')) {
    ++cursor;
  }

  if ((p2 == NULL) || (p2_length == 0)) {
    // remove the extension
    buffer[cursor] = '\0';
  } else {
    if (p2[0] != '.') {
      if (buffer[cursor] != '.') {
        string_append(p1, ".");
      } else {
        ++cursor;
      }
    }
    // insert the given extension.
    string_insert(p1, cursor, p2, p2_length);
  }
}

// void print_string(String *restrict s, FILE *restrict file) {
//   file_write(s->buffer, file);
// }