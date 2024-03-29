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

#include "imr/constants.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

Constants constants_create() {
  Constants constants;
  constants.length = 0;
  constants.capacity = 0;
  constants.buffer = NULL;
  return constants;
}

void constants_destroy(Constants *restrict constants) {
  assert(constants != NULL);

  if (constants->buffer == NULL) {
    return;
  }

  constants->length = 0;
  constants->capacity = 0;
  free(constants->buffer);
  constants->buffer = NULL;
}

size_t constants_append(Constants *restrict constants, Value value) {
  assert(constants != NULL);

  size_t sum;
  if (__builtin_add_overflow(constants->length, 1, &sum)) {
    panic("cannot allocate more than SIZE_MAX");
  }

  if (sum == __SIZE_MAX__) {
    panic("cannot allocate more than SIZE_MAX");
  }

  if (constants->capacity < sum) {
    size_t new_capacity = nearest_power_of_two(sum);

    size_t alloc_size;
    if (__builtin_mul_overflow(new_capacity, sizeof(Value), &alloc_size)) {
      panic("cannot allocate more than SIZE_MAX");
    }

    Value *result = realloc(constants->buffer, alloc_size);
    if (result == NULL) {
      panic_errno("realloc failed");
    }
    constants->buffer = result;
    constants->capacity = new_capacity;
  }

  size_t index = constants->length;
  constants->buffer[constants->length] = value;
  constants->length += 1;
  return index;
}

Value *constants_at(Constants *restrict constants, size_t index) {
  assert(constants != NULL);

  if (index >= constants->length) {
    panic("index out of bounds");
  }

  return constants->buffer + index;
}