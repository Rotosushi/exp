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

void string_init(string *restrict str) {
  str->length = 0;
  str->capacity = 0;
  str->buffer = NULL;
}

void string_reset(string *restrict str) {
  if ((str->length == 0) && (str->capacity == 0) && (str->buffer == NULL)) {
    return;
  }

  str->length = 0;
  str->capacity = 0;
  free(str->buffer);
  str->buffer = NULL;
}

string_view string_to_view(string const *restrict str) {
  string_view sv = {str->buffer, str->length};
  return sv;
}

void string_resize(string *restrict str, size_t capacity) {
  assert((capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  if (capacity < (str->capacity - 1)) {
    str->length = capacity;
    str->buffer[capacity] = '\0';
    return;
  }

  if (capacity == (str->capacity - 1)) {
    return;
  }

  // capacity > (str->capacity - 1)
  char *new_buffer = realloc(str->buffer, (capacity + 1) * sizeof(char));
  if (new_buffer == NULL) {
    panic_errno("realloc failed", sizeof("realloc failed"));
  }
  new_buffer[str->length] = '\0';

  str->buffer = new_buffer;
  str->capacity = capacity + 1;
}

void string_reserve_more(string *restrict str, size_t more_capacity) {
  assert((more_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");
  size_t sum_capacity;
  if (__builtin_add_overflow(str->capacity, more_capacity, &sum_capacity)) {
    panic("cannot allocate more than SIZE_MAX",
          sizeof("cannot allocate more than SIZE_MAX"));
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  char *new_buffer = realloc(str->buffer, (sum_capacity + 1) * sizeof(char));
  if (new_buffer == NULL) {
    panic_errno("realloc failed", sizeof("realloc failed"));
  }
  new_buffer[str->length] = '\0';

  str->buffer = new_buffer;
  str->capacity = sum_capacity + 1;
}

void string_assign(string *restrict str, const char *restrict data,
                   size_t data_length) {
  assert((data_length != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  string_reset(str);
  if (data_length == 0) {
    return;
  }

  // allocate space for at least 8 characters. to
  // avoid frequent reallocations of small size strings.
  size_t new_capacity = ulmax(data_length, 8);

  str->buffer = malloc((new_capacity + 1) * sizeof(char));
  if (str->buffer == NULL) {
    panic_errno("malloc failed", sizeof("malloc failed"));
  }
  memcpy(str->buffer, data, data_length);
  str->buffer[data_length] = '\0';
  str->length = data_length;
  str->capacity = new_capacity;
}

void string_append(string *restrict str, const char *restrict data,
                   size_t data_length) {
  if (data_length == 0) {
    return;
  }

  // if the data will fit in the existing capacity
  if ((str->capacity >= str->length) &&
      ((str->capacity - str->length) > data_length)) {
    memcpy(str->buffer + str->length, data, data_length);
    str->length += data_length;
    str->buffer[str->length] = '\0';
    return;
  }

  assert((data_length != SIZE_MAX) && "cannot allocate more than SIZE_MAX");
  size_t sum_capacity;
  if (__builtin_add_overflow(str->capacity, data_length, &sum_capacity)) {
    panic("cannot allocate more than SIZE_MAX",
          sizeof("cannot allocate more than SIZE_MAX"));
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  // grow the capacity of the string by a factor of two until
  // it is large enough to append <data>
  size_t new_capacity = nearest_power_of_two(sum_capacity);

  char *result = realloc(str->buffer, new_capacity + 1);
  if (result == NULL) {
    panic_errno("realloc failed", sizeof("realloc failed"));
  }
  str->buffer = result;

  memcpy(str->buffer + str->length, data, data_length);
  str->length += data_length;
  str->buffer[str->length] = '\0';
  str->capacity = new_capacity + 1;
}

void string_append_string(string *restrict s1, const string *restrict s2) {
  string_append(s1, s2->buffer, s2->length);
}

void string_append_char(string *restrict str, const char c) {
  char buffer[2];
  buffer[0] = c;
  buffer[1] = '\0';

  string_append(str, buffer, 1);
}