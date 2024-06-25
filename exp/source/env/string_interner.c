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
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/hash.h"
#include "utility/minmax.h"

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

  for (u64 i = 0; i < string_interner->capacity; ++i) {
    string_destroy(string_interner->buffer + i);
  }

  string_interner->capacity = 0;
  string_interner->count    = 0;
  deallocate(string_interner->buffer);
  string_interner->buffer = NULL;
}

static String *
string_interner_find(String *restrict strings, u64 capacity, StringView sv) {
  u64 index = hash_cstring(sv.ptr, sv.length) % capacity;
  while (1) {
    String *element = &(strings[index]);
    if ((string_empty(element)) || (string_eq(element, sv))) { return element; }

    index = (index + 1) % capacity;
  }
}

static void string_interner_grow(StringInterner *restrict string_interner) {
  Growth g         = array_growth_u64(string_interner->capacity, sizeof(String));
  String *elements = callocate(g.new_capacity, sizeof(String));

  // if the buffer isn't empty, we need to reinsert
  // all existing elements into the new buffer.
  if (string_interner->buffer != NULL) {
    for (u64 i = 0; i < string_interner->capacity; ++i) {
      String *element = &(string_interner->buffer[i]);
      if (string_empty(element)) { continue; }

      String *dest = string_interner_find(
          elements, g.new_capacity, string_to_view(element));
      string_assign_string(dest, element);
    }
    // this is allowed because we move all of the strings
    // to the new buffer.
    deallocate(string_interner->buffer);
  }

  string_interner->capacity = g.new_capacity;
  string_interner->buffer   = elements;
}

static bool string_interner_full(StringInterner *restrict string_interner) {
  u64 load_limit = (u64)floor((double)string_interner->capacity * STRING_INTERNER_MAX_LOAD);
  return (string_interner->count + 1) >= load_limit;
}

StringView string_interner_insert(StringInterner *restrict string_interner, StringView sv) {
  assert(string_interner != NULL);
  if (string_interner_full(string_interner)) { string_interner_grow(string_interner); }

  String *element = string_interner_find(string_interner->buffer, string_interner->capacity, sv);
  if (!string_empty(element)) { return string_to_view(element); }

  string_interner->count++;
  string_assign(element, sv);
  return string_to_view(element);
}