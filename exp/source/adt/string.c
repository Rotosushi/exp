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
  str.length   = 0;
  str.capacity = 0;
  str.buffer   = NULL;
  return str;
}

void string_destroy(String *restrict str) {
  assert(str != NULL);
  str->length   = 0;
  str->capacity = 0;
  free(str->buffer);
  str->buffer = NULL;
}

StringView string_to_view(String const *restrict str) {
  assert(str != NULL);
  StringView sv = string_view_from_string(str->buffer, str->length);
  return sv;
}

// StringView string_to_view_at(String const *restrict str, u64 offset,
//                              u64 length) {
//   assert(str != NULL);
//   assert(offset <= str->length);
//   assert((offset + length) <= str->length);

//   StringView sv = string_view_from_string(str->buffer + offset, length);
//   return sv;
// }

static void string_assign_impl(String *restrict str, char const *restrict ptr,
                               u64 length);

String string_from_view(StringView sv) {
  String string = string_create();
  string_assign_impl(&string, sv.ptr, sv.length);
  return string;
}

// String string_from_cstring(char const *restrict cs) {
//   String str = string_create();
//   string_assign_impl(&str, cs, strlen(cs));
//   return str;
// }

String string_from_file(FILE *restrict file) {
  String s = string_create();
  u64 flen = file_length(file);
  string_resize(&s, flen);
  file_read(s.buffer, flen, file);
  return s;
}

bool string_empty(String const *restrict string) {
  assert(string != NULL);
  return string->length == 0;
}

// i32 string_compare(String const *restrict s1, String const *restrict s2) {
//   assert(s1 != NULL);
//   assert(s2 != NULL);
//   // '\0' < any char
//   if (s1->length < s2->length) {
//     return -1;
//   } else if (s1->length > s2->length) {
//     // any char > '\0'
//     return 1;
//   } else {
//     return strncmp(s1->buffer, s2->buffer, s1->length);
//   }
// }

void string_resize(String *restrict str, u64 new_capacity) {
  assert(str != NULL);
  Growth g                 = array_growth_u64(new_capacity, sizeof(char));
  str->buffer              = reallocate(str->buffer, g.alloc_size);
  str->capacity            = g.new_capacity;
  str->buffer[str->length] = '\0';
}

// void string_reserve_more(String *restrict str, u64 more_capacity) {
//   assert(str != NULL);

//   u64 sum_capacity;
//   if (__builtin_add_overflow(str->capacity, more_capacity, &sum_capacity)) {
//     PANIC("cannot allocate more than SIZE_MAX");
//   }

//   string_resize(str, sum_capacity);
// }

static void string_assign_impl(String *restrict str, char const *restrict data,
                               u64 length) {
  string_destroy(str);
  u64 new_capacity = length + 1;
  str->buffer      = callocate(new_capacity, sizeof(char));
  str->length      = length;
  str->capacity    = new_capacity;
  memcpy(str->buffer, data, length);
}

void string_assign(String *restrict str, const char *restrict data) {
  assert(str != NULL);

  u64 data_length = strlen(data);

  string_assign_impl(str, data, data_length);
}

void string_assign_view(String *restrict str, StringView sv) {
  string_assign_impl(str, sv.ptr, sv.length);
}

// void string_move(String *restrict s1, String *restrict s2) {
//   string_destroy(s1);
//   s1->length   = s2->length;
//   s1->capacity = s2->capacity;
//   s1->buffer   = s2->buffer;

//   s2->buffer   = NULL;
//   s2->capacity = 0;
//   s2->length   = 0;
// }

static void string_append_impl(String *restrict str, char const *restrict data,
                               u64 size) {
  if (size == 0) {
    return;
  }

  if ((str->capacity - str->length) <= size) {
    string_resize(str, str->capacity + size);
  }

  memcpy(str->buffer + str->length, data, size);
  str->length += size;
  str->buffer[str->length] = '\0';
}

void string_append(String *restrict str, const char *restrict data) {
  assert(str != NULL);
  string_append_impl(str, data, strlen(data));
}

void string_append_sv(String *restrict str, StringView sv) {
  assert(str != NULL);
  string_append_impl(str, sv.ptr, sv.length);
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

// void string_append_view(String *restrict str, StringView sv) {
//   assert(str != NULL);
//   string_append_impl(str, sv.ptr, sv.length);
// }

// void string_append_string(String *restrict s1, const String *restrict s2) {
//   assert(s1 != NULL);
//   assert(s2 != NULL);
//   string_append(s1, s2->buffer);
// }

// void string_append_char(String *restrict str, const char c) {
//   assert(str != NULL);
//   char buffer[2];
//   buffer[0] = c;
//   buffer[1] = '\0';

//   string_append(str, buffer);
// }

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
    // 'erase' the entire buffer
    str->buffer[0] = '\0';
    str->length    = 0;
    return;
  }

  // erase <length> characters starting from <str->buffer + offset>
  char *pos       = str->buffer + offset;
  char *rest      = pos + length;
  u64 rest_length = (u64)((str->buffer + str->length) - rest);
  memmove(pos, rest, rest_length);
  u64 new_length          = offset + rest_length;
  str->buffer[new_length] = '\0';
  str->length             = new_length;
  return;
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
void string_insert(String *restrict str, u64 offset,
                   char const *restrict data) {
  assert(str != NULL);
  assert(offset <= str->length);
  u64 length = strlen(data);

  if ((offset + length) >= str->capacity) {
    string_resize(str, (offset + length));
    u64 added_length = (offset + length) - str->length;
    str->length += added_length;
  }

  memcpy(str->buffer + offset, data, length);
}

void string_replace_extension(String *restrict p1, const char *restrict p2) {
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
  char *buffer = p1->buffer;
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

  if (p2 == NULL) {
    // remove the extension
    char *pos       = buffer + cursor;
    char *rest      = pos + length;
    u64 rest_length = (u64)((buffer + length) - rest);
    string_erase(p1, cursor, rest_length);
  } else {
    if (p2[0] != '.') {
      if (buffer[cursor] != '.') {
        string_append(p1, ".");
      } else {
        ++cursor;
      }
    }
    // insert the given extension.
    string_insert(p1, cursor, p2);
  }
}

// void print_string(String *restrict s, FILE *restrict file) {
//   file_write(s->buffer, file);
// }