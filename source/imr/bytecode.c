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

#include "imr/bytecode.h"
#include "imr/opcode.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

Bytecode bytecode_create() {
  Bytecode bc;
  bc.length = 0;
  bc.capacity = 0;
  bc.buffer = NULL;
  return bc;
}

void bytecode_destroy(Bytecode *restrict bytecode) {
  assert(bytecode != NULL);

  if ((bytecode->buffer == NULL)) {
    bytecode->length = 0;
    bytecode->capacity = 0;
    return;
  }

  bytecode->length = 0;
  bytecode->capacity = 0;
  free(bytecode->buffer);
}

static void bytecode_emit_byte(Bytecode *restrict bytecode, uint8_t byte) {
  assert(bytecode != NULL);
  size_t sum_capacity;
  if (__builtin_add_overflow(bytecode->length, 1, &sum_capacity)) {
    panic("cannot allocate more than SIZE_MAX",
          sizeof("cannot allocate more than SIZE_MAX"));
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");

  if (bytecode->capacity < sum_capacity) {
    size_t new_capacity = nearest_power_of_two(sum_capacity);

    uint8_t *result = realloc(bytecode->buffer, new_capacity);
    if (result == NULL) {
      panic_errno("realloc failed", sizeof("realloc failed"));
    }
    bytecode->buffer = result;
    bytecode->capacity = new_capacity;
  }

  bytecode->buffer[bytecode->length] = byte;
  bytecode->length += 1;
}

void bytecode_emit_global_const(Bytecode *restrict bytecode, size_t name_index,
                                size_t value_index) {
  assert(name_index < 256 && "TODO");
  assert(value_index < 256 && "TODO");
  bytecode_emit_byte(bytecode, (uint8_t)OP_DEFINE_GLOBAL_CONST);
  bytecode_emit_byte(bytecode, (uint8_t)name_index);
  bytecode_emit_byte(bytecode, (uint8_t)value_index);
}