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
#include "utility/array_growth.h"
#include "utility/minmax.h"
#include "utility/string_hash.h"

#define STRING_INTERNER_MAX_LOAD 0.75

StringInterner string_interner_create() {
  StringInterner si;
  si.count    = 0;
  si.capacity = 0;
  si.buffer   = NULL;
  return si;
}

void string_interner_destroy(StringInterner *restrict si) {
  assert(si != NULL);

  if (si->buffer == NULL) {
    si->capacity = 0;
    si->count    = 0;
    return;
  }

  for (u64 i = 0; i < si->count; ++i) {
    string_destroy(si->buffer + i);
  }

  si->capacity = 0;
  si->count    = 0;
  free(si->buffer);
  si->buffer = NULL;
}

static String *string_interner_find(String *restrict strings, u64 capacity,
                                    char const *restrict buffer, u64 length) {
  u64 index = string_hash(buffer, length) % capacity;
  while (1) {
    String *element = &(strings[index]);
    if ((element->buffer == NULL) ||
        (strncmp(buffer, element->buffer, ulmin(length, element->length)) ==
         0)) {
      return element;
    }

    index = (index + 1) % capacity;
  }
}

static void string_interner_grow(StringInterner *restrict si) {
  Growth g         = array_growth(si->capacity, sizeof(String));
  String *elements = calloc(g.new_capacity, sizeof(String));

  // if the buffer isn't empty, we need to reinsert
  // all existing elements into the new buffer.
  if (si->buffer != NULL) {
    for (u64 i = 0; i < si->capacity; ++i) {
      String *element = &(si->buffer[i]);
      if (element->buffer == NULL) {
        continue;
      }

      String *dest   = string_interner_find(elements, g.new_capacity,
                                            element->buffer, element->length);
      dest->buffer   = element->buffer;
      dest->capacity = element->capacity;
      dest->length   = element->length;
    }
    // this is allowed because we move all of the strings
    // to the new buffer.
    free(si->buffer);
  }

  si->capacity = g.new_capacity;
  si->buffer   = elements;
}

static bool string_interner_full(StringInterner *restrict si) {
  u64 load_limit = (u64)floor((double)si->capacity * STRING_INTERNER_MAX_LOAD);
  return (si->count + 1) >= load_limit;
}

StringView string_interner_insert(StringInterner *restrict si, char const *data,
                                  u64 length) {
  assert(si != NULL);
  if (string_interner_full(si)) {
    string_interner_grow(si);
  }

  String *element =
      string_interner_find(si->buffer, si->capacity, data, length);
  if (element->buffer != NULL) {
    return string_to_view(element);
  }

  si->count++;
  StringView sv = string_view_from_string(data, length);
  string_assign_view(element, sv);
  return string_to_view(element);
}