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

#include "imr/registers.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

void registers_init(Registers *restrict registers) {
  assert(registers != NULL);
  registers->capacity = 0;
  registers->buffer   = NULL;
}

void registers_destroy(Registers *restrict registers) {
  assert(registers != NULL);
  registers->capacity = 0;
  free(registers->buffer);
  registers->buffer = NULL;
}

static void registers_grow(Registers *restrict registers, size_t capacity) {
  assert(registers != NULL);
  size_t new_capacity = nearest_power_of_two(capacity);

  size_t alloc_size;
  if (__builtin_mul_overflow(new_capacity, sizeof(Value), &alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  Value *result = realloc(registers->buffer, alloc_size);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }
  registers->buffer   = result;
  registers->capacity = new_capacity;
}

Value *registers_at(Registers *restrict registers, size_t index) {
  assert(registers != NULL);
  if (registers->capacity <= index) {
    registers_grow(registers, index);
  }

  return registers->buffer + index;
}
