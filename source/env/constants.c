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

#include "env/constants.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

Constants constants_create() {
  Constants constants;
  constants.length   = 0;
  constants.capacity = 0;
  constants.buffer   = NULL;
  return constants;
}

void constants_destroy(Constants *restrict constants) {
  assert(constants != NULL);
  constants->length   = 0;
  constants->capacity = 0;
  free(constants->buffer);
  constants->buffer = NULL;
}

static bool constants_full(Constants *restrict constants) {
  return constants->length == constants->capacity;
}

static void constants_grow(Constants *restrict constants) {
  u64 new_capacity = nearest_power_of_two(constants->capacity + 1);

  u64 alloc_size;
  if (__builtin_mul_overflow(new_capacity, sizeof(Value), &alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  Value *result = realloc(constants->buffer, alloc_size);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }
  constants->buffer   = result;
  constants->capacity = new_capacity;
}

// #TODO: this is the "greedy" approach, and it would be more
// space efficient if we only performed an insert when the
// new constant was unique. This trades time for space.
u64 constants_append(Constants *restrict constants, Value value) {
  assert(constants != NULL);

  if (constants_full(constants)) {
    constants_grow(constants);
  }

  u64 index                            = constants->length;
  constants->buffer[constants->length] = value;
  constants->length += 1;
  return index;
}

Value *constants_at(Constants *restrict constants, u64 index) {
  assert(constants != NULL);

  if (index >= constants->length) {
    PANIC("index out of bounds");
  }

  return constants->buffer + index;
}