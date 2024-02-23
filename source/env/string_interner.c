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
#include <stdlib.h>
#include <string.h>

#include "env/string_interner.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

StringInterner string_interner_create() {
  StringInterner string_interner;
  string_interner.length = 0;
  string_interner.capacity = 0;
  string_interner.buffer = NULL;
  return string_interner;
}

void string_interner_destroy(StringInterner *restrict string_interner) {
  assert(string_interner != NULL);

  if (string_interner->buffer == NULL) {
    string_interner->capacity = 0;
    string_interner->length = 0;
    return;
  }

  string_interner->capacity = 0;
  string_interner->length = 0;
  free(string_interner->buffer);
  string_interner->buffer = NULL;
}

StringView string_interner_insert(StringInterner *restrict string_interner,
                                  char const *data, size_t length) {
  assert(string_interner != NULL);

  // search for an existing string
  for (size_t i = 0; i < string_interner->length; ++i) {
    String *element = string_interner->buffer + i;
    if (length != element->length) {
      continue;
    }

    if (memcmp(data, element->buffer, length) == 0) {
      return string_to_view(element);
    }
  }

  // insert the new string
  size_t sum_capacity;
  if (__builtin_add_overflow(string_interner->length, 1, &sum_capacity)) {
    panic("cannot allocate more than SIZE_MAX",
          sizeof("cannot allocate more than SIZE_MAX"));
  }

  if (string_interner->capacity < sum_capacity) {
    size_t new_capacity = nearest_power_of_two(sum_capacity);

    size_t new_alloc_size;
    if (__builtin_mul_overflow(new_capacity, sizeof(String), &new_alloc_size)) {
      panic("cannot allocate more than SIZE_MAX",
            sizeof("cannot allocate more than SIZE_MAX"));
    }

    String *result = realloc(string_interner->buffer, new_alloc_size);
    if (result == NULL) {
      panic_errno("realloc failed", sizeof("realloc_failed"));
    }
    string_interner->buffer = result;
    string_interner->capacity = new_capacity;
  }

  String *new_str = &(string_interner->buffer[string_interner->length]);
  string_interner->length += 1;
  *new_str = string_create();
  string_assign(new_str, data, length);
  return string_to_view(new_str);
}