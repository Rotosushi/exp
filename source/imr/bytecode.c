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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "imr/bytecode.h"
#include "imr/opcode.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

Bytecode bytecode_create() {
  Bytecode bc;
  bc.length   = 0;
  bc.capacity = 0;
  bc.buffer   = NULL;
  return bc;
}

void bytecode_destroy(Bytecode *restrict bytecode) {
  assert(bytecode != NULL);
  bytecode->length   = 0;
  bytecode->capacity = 0;
  free(bytecode->buffer);
}

void bytecode_clone(Bytecode *target, Bytecode *source) {
  assert(target != NULL);
  assert(source != NULL);
  if (target == source) {
    return;
  }

  bytecode_destroy(target);
  target->length   = source->length;
  target->capacity = source->capacity;

  u8 *result = realloc(target->buffer, target->capacity);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }
  target->buffer = result;

  memcpy(target->buffer, source->buffer, target->length);
}

bool bytecode_equality(Bytecode *b1, Bytecode *b2) {
  assert(b1 != NULL);
  assert(b2 != NULL);
  if (b1 == b2) {
    return 1;
  }

  return memcmp(b1->buffer, b2->buffer, b1->length) == 0;
}

static bool bytecode_full(Bytecode *restrict bytecode) {
  u64 sum_capacity;
  if (__builtin_add_overflow(bytecode->length, 1, &sum_capacity)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }
  assert((sum_capacity != SIZE_MAX) && "cannot allocate more than SIZE_MAX");
  return bytecode->capacity < sum_capacity;
}

static void bytecode_grow(Bytecode *restrict bytecode) {
  u64 new_capacity = nearest_power_of_two(bytecode->capacity + 1);

  u8 *result = realloc(bytecode->buffer, new_capacity);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }
  bytecode->buffer   = result;
  bytecode->capacity = new_capacity;
}

static void bytecode_emit_byte(Bytecode *restrict bytecode, u8 byte) {
  assert(bytecode != NULL);

  if (bytecode_full(bytecode)) {
    bytecode_grow(bytecode);
  }

  bytecode->buffer[bytecode->length] = byte;
  bytecode->length += 1;
}

static void bytecode_emit_immediate(Bytecode *restrict bytecode, u64 immediate,
                                    u64 bytes) {
  assert((bytes == sizeof(u8)) || (bytes == sizeof(uint16_t)) ||
         (bytes == sizeof(uint32_t)) || (bytes == sizeof(uint64_t)));
  for (u64 i = 0, j = bytes; i < bytes; ++i, --j) {
    u64 shift = (j * 8) - 8;
    u8 byte   = (immediate >> shift) & 0xFF;
    bytecode_emit_byte(bytecode, byte);
  }
}

u64 bytecode_read_immediate(Bytecode *restrict bytecode, u64 offset,
                            u64 bytes) {
  assert((bytes == sizeof(u8)) || (bytes == sizeof(uint16_t)) ||
         (bytes == sizeof(uint32_t)) || (bytes == sizeof(uint64_t)));
  assert((offset + bytes) <= bytecode->length);
  u64 result = 0;
  for (u64 i = 0, j = bytes; i < bytes; ++i, --j) {
    u64 shift = (j * 8) - 8;
    u8 byte   = bytecode->buffer[offset + i];
    result |= (((u64)byte) << shift);
  }
  return result;
}

void bytecode_emit_stop(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_STOP);
}

void bytecode_emit_return(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_RETURN);
}

void bytecode_emit_push_constant(Bytecode *restrict bytecode, u64 index) {
  if (index <= UINT8_MAX) {
    bytecode_emit_byte(bytecode, (u8)OP_PUSH_CONSTANT_U8);
    bytecode_emit_immediate(bytecode, index, sizeof(u8));
  } else if (index <= UINT16_MAX) {
    bytecode_emit_byte(bytecode, (u8)OP_PUSH_CONSTANT_U16);
    bytecode_emit_immediate(bytecode, index, sizeof(uint16_t));
  } else if (index <= UINT32_MAX) {
    bytecode_emit_byte(bytecode, (u8)OP_PUSH_CONSTANT_U32);
    bytecode_emit_immediate(bytecode, index, sizeof(uint32_t));
  } else {
    bytecode_emit_byte(bytecode, (u8)OP_PUSH_CONSTANT_U64);
    bytecode_emit_immediate(bytecode, index, sizeof(uint64_t));
  }
  // #NOTE:
  // iff the index to the constant somehow is >= UINT64_MAX then
  // it is going to overflow the u64 it is being passed in as.
  // this is clearly a defect. but cannot be caught at this point
  // in execution because we are downstream of the overflow.
}

// void bytecode_emit_push_register(Bytecode *restrict bytecode,
//                                  u8 register_index) {
//   bytecode_emit_byte(bytecode, (u8)OP_PUSH_REGISTER);
//   bytecode_emit_byte(bytecode, register_index);
// }

void bytecode_emit_pop(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_POP);
}

// void bytecode_emit_pop_register(Bytecode *restrict bytecode,
//                                 u8 register_index) {
//   bytecode_emit_byte(bytecode, (u8)OP_POP_REGISTER);
//   bytecode_emit_byte(bytecode, register_index);
// }

// void bytecode_emit_move_constant_to_register(Bytecode *restrict bytecode,
//                                              u8 register_index,
//                                              u64 constant_index) {
//   bytecode_emit_byte(bytecode, (u8)OP_MOVE_CONSTANT_TO_REGISTER);
//   bytecode_emit_byte(bytecode, register_index);
//   bytecode_emit_byte(bytecode, (u8)constant_index);
// }

void bytecode_emit_define_global_constant(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_DEFINE_GLOBAL_CONSTANT);
}

void bytecode_emit_unop_minus(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_UNOP_MINUS);
}

void bytecode_emit_binop_plus(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_BINOP_PLUS);
}

void bytecode_emit_binop_minus(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_BINOP_MINUS);
}

void bytecode_emit_binop_star(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_BINOP_STAR);
}

void bytecode_emit_binop_slash(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_BINOP_SLASH);
}

void bytecode_emit_binop_percent(Bytecode *restrict bytecode) {
  bytecode_emit_byte(bytecode, (u8)OP_BINOP_PERCENT);
}
