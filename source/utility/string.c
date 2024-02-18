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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utility/string.h"

void string_init(string *str) {
  str->length = 0;
  str->capacity = 0;
  str->buffer = NULL;
}

void string_reset(string *str) {
  if ((str->length == 0) && (str->capacity == 0) && (str->buffer == NULL)) {
    return;
  }

  str->length = 0;
  str->capacity = 0;
  free(str->buffer);
  str->buffer = NULL;
}

string_view string_to_view(string const *str) {
  string_view sv = {str->buffer, str->length};
  return sv;
}

void string_resize(string *str, size_t capacity) {
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
    perror("realloc failed");
    exit(EXIT_FAILURE);
  }
  new_buffer[str->length] = '\0';

  str->buffer = new_buffer;
  str->capacity = capacity + 1;
}

void string_reserve_more(string *str, size_t more_capacity) {
  assert((more_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");
  size_t sum_capacity;
  if (__builtin_add_overflow(str->capacity, more_capacity, &sum_capacity)) {
    fputs("cannot allocate more than SIZE_MAX\n", stderr);
    exit(EXIT_FAILURE);
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  char *new_buffer = realloc(str->buffer, (sum_capacity + 1) * sizeof(char));
  if (new_buffer == NULL) {
    perror("realloc failed");
    exit(EXIT_FAILURE);
  }
  new_buffer[str->length] = '\0';

  str->buffer = new_buffer;
  str->capacity = sum_capacity + 1;
}

static size_t ulmax(size_t a, size_t b) { return (a > b) ? a : b; }

void string_assign(string *str, const char *data, size_t data_length) {
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
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  memcpy(str->buffer, data, data_length);
  str->buffer[data_length] = '\0';
  str->length = data_length;
  str->capacity = new_capacity;
}

/**
 * @brief compute the nearest power of 2 greater than
 * or equal to value.
 *
 * @param value
 * @return size_t
 */
static size_t nearest_power_of_two(size_t value) {
  size_t accumulator = 8, scale_factor = 2;
  while (1) {
    if (accumulator >= value) {
      return accumulator;
    }
    // if scaling by an additional power of two would overflow
    // a size_t, clamp the result at SIZE_MAX - 1.
    if (__builtin_mul_overflow(accumulator, scale_factor, &accumulator)) {
      return SIZE_MAX - 1;
    }
  }
}

void string_append(string *str, const char *data, size_t data_length) {
  if (data_length == 0) {
    return;
  }

  // if the data will fit in the existing capacity
  if ((str->capacity - str->length) > data_length) {
    memcpy(str->buffer + str->length, data, data_length);
    str->length += data_length;
    str->buffer[str->length] = '\0';
    return;
  }

  assert((data_length != SIZE_MAX) && "cannot allocate more than SIZE_MAX");
  size_t sum_capacity;
  if (__builtin_add_overflow(str->capacity, data_length, &sum_capacity)) {
    fputs("cannot allocate more than SIZE_MAX\n", stderr);
    exit(EXIT_FAILURE);
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  // grow the capacity of the string by a factor of two until
  // it is large enough to append <data>
  size_t new_capacity = nearest_power_of_two(sum_capacity);

  char *result = realloc(str->buffer, new_capacity + 1);
  if (result == NULL) {
    perror("realloc failed");
    exit(EXIT_FAILURE);
  }
  str->buffer = result;

  memcpy(str->buffer + str->length, data, data_length);
  str->length += data_length;
  str->buffer[str->length] = '\0';
  str->capacity = new_capacity + 1;
}

void string_concat(string *s1, string const *s2) {
  string_append(s1, s2->buffer, s2->length);
}