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

#include "utility/minmax.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"
#include "utility/string.h"

String string_create() {
  String str;
  str.length = 0;
  str.capacity = 0;
  str.buffer = NULL;
  return str;
}

void string_destroy(String *restrict str) {
  assert(str != NULL);
  if ((str->length == 0) && (str->capacity == 0) && (str->buffer == NULL)) {
    return;
  }

  str->length = 0;
  str->capacity = 0;

  free(str->buffer);
  str->buffer = NULL;
}

static void string_assign_impl(String *restrict str, char const *restrict data,
                               size_t length) {
  string_resize(str, length);
  memcpy(str->buffer, data, length);
  str->buffer[length] = '\0';
  str->length = length;
}

StringView string_to_view(String const *restrict str) {
  assert(str != NULL);
  StringView sv = {str->buffer, str->length};
  return sv;
}

StringView string_to_view_at(String const *restrict str, size_t offset,
                             size_t length) {
  assert(str != NULL);
  assert(offset <= str->length);
  assert((offset + length) <= str->length);

  StringView sv = {str->buffer + offset, length};
  return sv;
}

String string_from_view(StringView sv) {
  String string;
  string_assign_impl(&string, sv.ptr, sv.length);
  return string;
}

void print_string(String *string, FILE *file) { fputs(string->buffer, file); }

bool string_empty(String const *restrict string) {
  assert(string != NULL);
  return string->length == 0;
}

int string_compare(String const *restrict s1, String const *restrict s2) {
  assert(s1 != NULL);
  assert(s2 != NULL);
  // '\0' < any char
  if (s1->length < s2->length) {
    return -1;
  } else if (s1->length > s2->length) {
    // any char > '\0'
    return 1;
  } else {
    return strncmp(s1->buffer, s2->buffer, s1->length);
  }
}

void string_resize(String *restrict str, size_t capacity) {
  assert(str != NULL);
  assert((capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  // "resize" to the same size means we can exit early.
  if ((str->capacity > 0) && (capacity == (str->capacity - 1))) {
    return;
  }

  char *new_buffer = realloc(str->buffer, (capacity + 1) * sizeof(char));
  if (new_buffer == NULL) {
    panic_errno("realloc failed");
  }

  // the new buffer is smaller than it once was
  if (str->length > capacity) {
    str->length = capacity;
  }

  str->buffer = new_buffer;
  str->capacity = capacity + 1;
  str->buffer[str->length] = '\0';
}

void string_reserve_more(String *restrict str, size_t more_capacity) {
  assert(str != NULL);
  assert((more_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");
  size_t sum_capacity;
  if (__builtin_add_overflow(str->capacity, more_capacity, &sum_capacity)) {
    panic("cannot allocate more than SIZE_MAX");
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  size_t new_capacity = nearest_power_of_two(sum_capacity);

  char *new_buffer = realloc(str->buffer, new_capacity * sizeof(char));
  if (new_buffer == NULL) {
    panic_errno("realloc failed");
  }

  str->buffer = new_buffer;
  str->capacity = new_capacity;
}

void string_assign(String *restrict str, const char *restrict data) {
  assert(str != NULL);

  size_t data_length = strlen(data);

  string_assign_impl(str, data, data_length);
}

void string_assign_view(String *restrict str, StringView sv) {
  string_assign_impl(str, sv.ptr, sv.length);
}

static void string_append_impl(String *restrict str, char const *restrict data,
                               size_t size) {
  if (size == 0) {
    return;
  }

  if ((str->capacity - str->length) <= size) {
    string_reserve_more(str, size);
  }

  memcpy(str->buffer + str->length, data, size);
  str->length += size;
  str->buffer[str->length] = '\0';
}

void string_append(String *restrict str, const char *restrict data) {
  assert(str != NULL);
  string_append_impl(str, data, strlen(data));
}

void string_append_view(String *restrict str, StringView sv) {
  assert(str != NULL);
  string_append_impl(str, sv.ptr, sv.length);
}

void string_append_string(String *restrict s1, const String *restrict s2) {
  assert(s1 != NULL);
  assert(s2 != NULL);
  string_append(s1, s2->buffer);
}

void string_append_char(String *restrict str, const char c) {
  assert(str != NULL);
  char buffer[2];
  buffer[0] = c;
  buffer[1] = '\0';

  string_append(str, buffer);
}

/*
  essentially can be broken into these cases

  1 offset == 0, length == str->length

  2 offset == 0, length < str->length

  3 offset > 0, (offset + length) == str->length

  4 offset > 0, (offset + length) < str->length
*/
void string_erase(String *restrict str, size_t offset, size_t length) {
  assert(str != NULL);
  assert(offset <= str->length);
  assert((offset + length) <= str->length);

  if ((offset == 0) && (length == str->length)) {
    // 'erase' the entire buffer
    str->buffer[0] = '\0';
    str->length = 0;
    return;
  }

  // erase <length> characters starting from <str->buffer + offset>
  char *pos = str->buffer + offset;
  char *rest = pos + length;
  size_t rest_length = (size_t)((str->buffer + str->length) - rest);
  memmove(pos, rest, rest_length);
  size_t new_length = offset + rest_length;
  str->buffer[new_length] = '\0';
  str->length = new_length;
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
void string_insert(String *restrict str, size_t offset,
                   char const *restrict data) {
  assert(str != NULL);
  assert(offset <= str->length);
  size_t length = strlen(data);

  if ((offset + length) >= str->capacity) {
    string_resize(str, (offset + length));
    size_t added_length = (offset + length) - str->length;
    str->length += added_length;
  }

  memcpy(str->buffer + offset, data, length);
}