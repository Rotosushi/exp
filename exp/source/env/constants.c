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
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/numeric_conversions.h"
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
  return (constants->length + 1) >= constants->capacity;
}

static void constants_grow(Constants *restrict constants) {
  Growth g            = array_growth_u64(constants->capacity, sizeof(Value));
  constants->buffer   = reallocate(constants->buffer, g.alloc_size);
  constants->capacity = g.new_capacity;
}

[[maybe_unused]] static bool index_inbounds(Constants *restrict c, u16 i) {
  return i < c->length;
}

Operand constants_add(Constants *restrict c, Value value) {
  assert(c != NULL);

  for (u64 i = 0; i < c->length; ++i) {
    Value *v = c->buffer + i;
    if (value_equality(v, &value)) {
      if (i > u16_MAX) {
        PANIC("constant index out of bounds");
      }
      return opr_constant((u16)i);
    }
  }

  if (constants_full(c)) {
    constants_grow(c);
  }

  u64 index            = c->length;
  c->buffer[c->length] = value;
  c->length += 1;

  if (index > u16_MAX) {
    PANIC("constant index out of bounds");
  }

  return opr_constant((u16)index);
}

Value *constants_at(Constants *restrict c, u16 i) {
  assert(c != NULL);
  assert(index_inbounds(c, i));
  return c->buffer + i;
}

void print_constants(Constants const *restrict c, FILE *restrict file) {
  for (u64 i = 0; i < c->length; ++i) {
    print_u64(i, RADIX_DECIMAL, file);
    file_write(": ", file);
    file_write("[", file);
    print_value(c->buffer + i, file);
    file_write("]\n", file);
  }
}