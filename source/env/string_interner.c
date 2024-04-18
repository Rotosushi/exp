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
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "env/string_interner.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"
#include "utility/string_hash.h"

#define STRING_INTERNER_MAX_LOAD 0.75

StringInterner string_interner_create() {
  StringInterner string_interner;
  string_interner.count    = 0;
  string_interner.capacity = 0;
  string_interner.buffer   = NULL;
  return string_interner;
}

void string_interner_destroy(StringInterner *restrict string_interner) {
  assert(string_interner != NULL);

  if (string_interner->buffer == NULL) {
    string_interner->capacity = 0;
    string_interner->count    = 0;
    return;
  }

  for (u64 i = 0; i < string_interner->count; ++i) {
    string_destroy(string_interner->buffer + i);
  }

  string_interner->capacity = 0;
  string_interner->count    = 0;
  free(string_interner->buffer);
  string_interner->buffer = NULL;
}

static String *string_interner_find(String *restrict strings, u64 capacity,
                                    char const *restrict buffer, u64 length) {
  u64 index = string_hash(buffer, length) % capacity;
  while (1) {
    String *element = &(strings[index]);
    if ((element->buffer == NULL) ||
        (strncmp(buffer, element->buffer, length) == 0)) {
      return element;
    }

    index = (index + 1) % capacity;
  }
}

static void string_interner_grow(StringInterner *restrict string_interner,
                                 u64 capacity) {
  String *elements = calloc(capacity, sizeof(String));

  // if the buffer isn't empty, we need to reinsert
  // all existing elements into the new buffer.
  if (string_interner->buffer != NULL) {
    for (u64 i = 0; i < string_interner->capacity; ++i) {
      String *element = &(string_interner->buffer[i]);
      if (element->buffer == NULL) {
        continue;
      }

      String *dest   = string_interner_find(elements, capacity, element->buffer,
                                            element->length);
      dest->buffer   = element->buffer;
      dest->capacity = element->capacity;
      dest->length   = element->length;
    }

    free(string_interner->buffer);
  }

  string_interner->capacity = capacity;
  string_interner->buffer   = elements;
}

static bool string_interner_full(StringInterner *restrict string_interner) {
  u64 new_count;
  if (__builtin_add_overflow(string_interner->count, 1, &new_count)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  u64 load_limit =
      (u64)floor((double)string_interner->capacity * STRING_INTERNER_MAX_LOAD);
  return new_count >= load_limit;
}

StringView string_interner_insert(StringInterner *restrict string_interner,
                                  char const *data, u64 length) {
  assert(string_interner != NULL);
  if (string_interner_full(string_interner)) {
    u64 capacity = nearest_power_of_two(string_interner->capacity + 1);
    string_interner_grow(string_interner, capacity);
  }

  String *element = string_interner_find(
      string_interner->buffer, string_interner->capacity, data, length);
  if (element->buffer != NULL) {
    return string_to_view(element);
  }

  string_interner->count++;
  StringView sv = string_view_from_string(data, length);
  string_assign_view(element, sv);
  return string_to_view(element);
}