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
  bytecode->buffer = NULL;
}

static bool bytecode_full(Bytecode *restrict bytecode) {
  return bytecode->capacity <= (bytecode->length + 1);
}

static void bytecode_grow(Bytecode *restrict bytecode) {
  u64 new_capacity = nearest_power_of_two(bytecode->capacity + 1);

  Instruction *result = realloc(bytecode->buffer, new_capacity);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }
  bytecode->buffer   = result;
  bytecode->capacity = new_capacity;
}

static void bytecode_emit_instruction(Bytecode *restrict bytecode,
                                      Instruction I) {
  if (bytecode_full(bytecode)) {
    bytecode_grow(bytecode);
  }

  bytecode->buffer[bytecode->length] = I;
  bytecode->length += 1;
}

void bytecode_emit_load_immediate(Bytecode *restrict bc, u16 A, u32 imm) {
  Instruction I = 0;
  INST_SET_OP(I, OP_LOADI);
  INST_SET_A(I, A);
  INST_SET_Bx(I, imm);
  bytecode_emit_instruction(bc, I);
}

void bytecode_emit_neg(Bytecode *restrict bc, u16 A, u16 B) {
  Instruction I = 0;
  INST_SET_OP(I, OP_NEG);
  INST_SET_A(I, A);
  INST_SET_B(I, B);
  bytecode_emit_instruction(bc, I);
}

void bytecode_emit_add(Bytecode *restrict bc, u16 A, u16 B, u16 C) {
  Instruction I = 0;
  INST_SET_OP(I, OP_ADD);
  INST_SET_A(I, A);
  INST_SET_B(I, B);
  INST_SET_C(I, C);
  bytecode_emit_instruction(bc, I);
}

void bytecode_emit_sub(Bytecode *restrict bc, u16 A, u16 B, u16 C) {
  Instruction I = 0;
  INST_SET_OP(I, OP_SUB);
  INST_SET_A(I, A);
  INST_SET_B(I, B);
  INST_SET_C(I, C);
  bytecode_emit_instruction(bc, I);
}

void bytecode_emit_mul(Bytecode *restrict bc, u16 A, u16 B, u16 C) {
  Instruction I = 0;
  INST_SET_OP(I, OP_MUL);
  INST_SET_A(I, A);
  INST_SET_B(I, B);
  INST_SET_C(I, C);
  bytecode_emit_instruction(bc, I);
}

void bytecode_emit_div(Bytecode *restrict bc, u16 A, u16 B, u16 C) {
  Instruction I = 0;
  INST_SET_OP(I, OP_DIV);
  INST_SET_A(I, A);
  INST_SET_B(I, B);
  INST_SET_C(I, C);
  bytecode_emit_instruction(bc, I);
}

void bytecode_emit_mod(Bytecode *restrict bc, u16 A, u16 B, u16 C) {
  Instruction I = 0;
  INST_SET_OP(I, OP_MOD);
  INST_SET_A(I, A);
  INST_SET_B(I, B);
  INST_SET_C(I, C);
  bytecode_emit_instruction(bc, I);
}