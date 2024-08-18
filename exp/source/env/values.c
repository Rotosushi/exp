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

#include "env/values.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

Values values_create() {
  Values values;
  values.length   = 0;
  values.capacity = 0;
  values.buffer   = NULL;
  return values;
}

void values_destroy(Values *restrict values) {
  assert(values != NULL);
  for (u64 i = 0; i < values->length; ++i) {
    Value *constant = values->buffer + i;
    value_destroy(constant);
  }

  values->length   = 0;
  values->capacity = 0;
  deallocate(values->buffer);
  values->buffer = NULL;
}

static bool values_full(Values *restrict values) {
  return (values->length + 1) >= values->capacity;
}

static void values_grow(Values *restrict values) {
  Growth g         = array_growth_u64(values->capacity, sizeof(Value));
  values->buffer   = reallocate(values->buffer, g.alloc_size);
  values->capacity = g.new_capacity;
}

[[maybe_unused]] static bool index_inbounds(Values *restrict values,
                                            u64 index) {
  return index < values->length;
}

Operand values_add(Values *restrict values, Value value) {
  assert(values != NULL);

  for (u64 i = 0; i < values->length; ++i) {
    if (i == u64_MAX) { PANIC("constant index out of bounds"); }
    Value *v = values->buffer + i;
    if (value_equality(v, &value)) { return operand_constant(i); }
  }

  if (values_full(values)) { values_grow(values); }

  u64 index                      = values->length;
  values->buffer[values->length] = value;
  values->length += 1;

  return operand_constant(index);
}

Value *values_at(Values *restrict values, u64 index) {
  assert(values != NULL);
  assert(index_inbounds(values, index));
  return values->buffer + index;
}

void print_values(Values const *restrict values, FILE *restrict file) {
  for (u64 i = 0; i < values->length; ++i) {
    print_u64(i, file);
    file_write(": ", file);
    file_write("[", file);
    print_value(values->buffer + i, file);
    file_write("]\n", file);
  }
}
