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

#include "env/context.h"
#include "env/values.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

Values values_create() {
  Values values;
  values.count    = 0;
  values.capacity = 0;
  values.buffer   = NULL;
  return values;
}

void values_destroy(Values *restrict values) {
  assert(values != NULL);
  for (u64 i = 0; i < values->count; ++i) {
    Value *constant = values->buffer + i;
    value_destroy(constant);
  }

  values->count    = 0;
  values->capacity = 0;
  deallocate(values->buffer);
  values->buffer = NULL;
}

static bool values_full(Values *restrict values) {
  return (values->count + 1) >= values->capacity;
}

static void values_grow(Values *restrict values) {
  Growth g         = array_growth_u16(values->capacity, sizeof(Value));
  values->buffer   = reallocate(values->buffer, g.alloc_size);
  values->capacity = (u16)g.new_capacity;
}

Operand values_add(Values *restrict values, Value value) {
  assert(values != NULL);

  for (u16 i = 0; i < values->count; ++i) {
    Value *v = values->buffer + i;
    if (value_equality(v, &value)) {
      value_destroy(&value);
      return operand_constant(i);
    }
  }

  if (values_full(values)) { values_grow(values); }

  u16 index                     = values->count;
  values->buffer[values->count] = value;
  values->count += 1;

  return operand_constant(index);
}

Value *values_at(Values *restrict values, u16 index) {
  assert(values != NULL);
  assert(index < values->count);
  return values->buffer + index;
}

void print_values(Values const *restrict values,
                  FILE *restrict file,
                  Context *restrict context) {
  for (u16 i = 0; i < values->count; ++i) {
    file_write_u64(i, file);
    file_write(": ", file);
    file_write("[", file);
    print_value(values->buffer + i, file, context);
    file_write("]\n", file);
  }
}
