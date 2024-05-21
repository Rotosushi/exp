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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>

#include "backend/x64/bytecode.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

x64_Bytecode x64_bytecode_create() {
  x64_Bytecode bc = {.length = 0, .capacity = 0, .buffer = NULL};
  return bc;
}

void x64_bytecode_destroy(x64_Bytecode *restrict bc) {
  assert(bc != NULL);
  bc->length   = 0;
  bc->capacity = 0;
  free(bc->buffer);
  bc->buffer = NULL;
}

static bool x64bytecode_full(x64_Bytecode *restrict bc) {
  return (bc->length + 1) >= bc->capacity;
}

static void x64bytecode_grow(x64_Bytecode *restrict bc) {
  Growth g     = array_growth_u16(bc->capacity, sizeof(x64_Instruction));
  bc->buffer   = reallocate(bc->buffer, g.alloc_size);
  bc->capacity = (u16)g.new_capacity;
}

static void x64bytecode_append_instruction(x64_Bytecode *restrict bc,
                                           x64_Instruction I) {
  if (x64bytecode_full(bc)) { x64bytecode_grow(bc); }

  bc->buffer[bc->length] = I;
  bc->length += 1;
}

static void x64bytecode_prepend_instruction(x64_Bytecode *restrict bc,
                                            x64_Instruction I) {
  if (x64bytecode_full(bc)) { x64bytecode_grow(bc); }

  // move all instructions forward one location
  for (u16 idx = bc->length; idx > 0; --idx) {
    bc->buffer[idx] = bc->buffer[idx - 1];
  }

  bc->buffer[0] = I;
  bc->length += 1;
}

void x64_bytecode_append_ret(x64_Bytecode *restrict bc) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst(X64OPC_RET);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_append_push(x64_Bytecode *restrict bc, x64_Operand A) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_A(X64OPC_PUSH, A);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_prepend_push(x64_Bytecode *restrict bc, x64_Operand A) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_A(X64OPC_PUSH, A);
  x64bytecode_prepend_instruction(bc, I);
}

void x64_bytecode_append_pop(x64_Bytecode *restrict bc, x64_Operand A) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_A(X64OPC_POP, A);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_append_mov(x64_Bytecode *restrict bc,
                             x64_Operand A,
                             x64_Operand B) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_AB(X64OPC_MOV, A, B);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_prepend_mov(x64_Bytecode *restrict bc,
                              x64_Operand A,
                              x64_Operand B) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_AB(X64OPC_MOV, A, B);
  x64bytecode_prepend_instruction(bc, I);
}

void x64_bytecode_append_neg(x64_Bytecode *restrict bc, x64_Operand A) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_A(X64OPC_NEG, A);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_append_add(x64_Bytecode *restrict bc,
                             x64_Operand A,
                             x64_Operand B) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_AB(X64OPC_ADD, A, B);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_append_sub(x64_Bytecode *restrict bc,
                             x64_Operand A,
                             x64_Operand B) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_AB(X64OPC_SUB, A, B);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_prepend_sub(x64_Bytecode *restrict bc,
                              x64_Operand A,
                              x64_Operand B) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_AB(X64OPC_SUB, A, B);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_append_imul(x64_Bytecode *restrict bc, x64_Operand A) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_A(X64OPC_IMUL, A);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_append_idiv(x64_Bytecode *restrict bc, x64_Operand A) {
  assert(bc != NULL);
  x64_Instruction I = x64_inst_A(X64OPC_IDIV, A);
  x64bytecode_append_instruction(bc, I);
}

void x64_bytecode_emit(x64_Bytecode *restrict bc,
                       String *restrict buffer,
                       Context *restrict context) {
  for (u16 i = 0; i < bc->length; ++i) {
    x64_inst_emit(bc->buffer[i], buffer, context);
    string_append(buffer, SV("\n"));
  }
}