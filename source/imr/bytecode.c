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

static bool bytecode_full(Bytecode *restrict bytecode) {
  size_t sum_capacity;
  if (__builtin_add_overflow(bytecode->length, 1, &sum_capacity)) {
    panic("cannot allocate more than SIZE_MAX");
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");
  return bytecode->capacity < sum_capacity;
}

static void bytecode_grow(Bytecode *restrict bytecode) {
  size_t new_capacity = nearest_power_of_two(bytecode->capacity + 1);

  uint8_t *result = realloc(bytecode->buffer, new_capacity);
  if (result == NULL) {
    panic_errno("realloc failed");
  }
  bytecode->buffer = result;
  bytecode->capacity = new_capacity;
}

static void bytecode_emit_byte(Bytecode *restrict bytecode, uint8_t byte) {
  assert(bytecode != NULL);

  if (bytecode_full(bytecode)) {
    bytecode_grow(bytecode);
  }

  bytecode->buffer[bytecode->length] = byte;
  bytecode->length += 1;
}

void bytecode_emit_stop(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (uint8_t)OP_STOP);
}

void bytecode_emit_push_constant(Bytecode *restrict bytecode,
                                 size_t value_index) {
  assert(value_index < 256 && "TODO");
  bytecode_emit_byte(bytecode, (uint8_t)OP_PUSH_CONSTANT);
  bytecode_emit_byte(bytecode, (uint8_t)value_index);
}

// void bytecode_emit_push_register(Bytecode *restrict bytecode,
//                                  uint8_t register_index) {
//   bytecode_emit_byte(bytecode, (uint8_t)OP_PUSH_REGISTER);
//   bytecode_emit_byte(bytecode, register_index);
// }

void bytecode_emit_pop(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (uint8_t)OP_POP);
}

// void bytecode_emit_pop_register(Bytecode *restrict bytecode,
//                                 uint8_t register_index) {
//   bytecode_emit_byte(bytecode, (uint8_t)OP_POP_REGISTER);
//   bytecode_emit_byte(bytecode, register_index);
// }

// void bytecode_emit_move_constant_to_register(Bytecode *restrict bytecode,
//                                              uint8_t register_index,
//                                              size_t constant_index) {
//   bytecode_emit_byte(bytecode, (uint8_t)OP_MOVE_CONSTANT_TO_REGISTER);
//   bytecode_emit_byte(bytecode, register_index);
//   bytecode_emit_byte(bytecode, (uint8_t)constant_index);
// }

void bytecode_emit_define_global_constant(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (uint8_t)OP_DEFINE_GLOBAL_CONSTANT);
}