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

#include "backend/x64_bytecode.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

X64Bytecode x64bytecode_create() {
  X64Bytecode bc = {.length = 0, .capacity = 0, .buffer = NULL};
  return bc;
}

void x64bytecode_destroy(X64Bytecode *restrict bc) {
  assert(bc != NULL);
  bc->length   = 0;
  bc->capacity = 0;
  free(bc->buffer);
  bc->buffer = NULL;
}

static bool x64bytecode_full(X64Bytecode *restrict bc) {
  return (bc->length + 1) >= bc->capacity;
}

static void x64bytecode_grow(X64Bytecode *restrict bc) {
  Growth g     = array_growth_u16(bc->capacity, sizeof(X64Instruction));
  bc->buffer   = reallocate(bc->buffer, g.alloc_size);
  bc->capacity = (u16)g.new_capacity;
}

static void x64bytecode_emit(X64Bytecode *restrict bc, X64Instruction I) {
  if (x64bytecode_full(bc)) {
    x64bytecode_grow(bc);
  }

  bc->buffer[bc->length] = I;
  bc->length += 1;
}

void x64bytecode_emit_ret(X64Bytecode *restrict bc) {
  assert(bc != NULL);
  X64Instruction I = x64inst(X64OPC_RET);
  x64bytecode_emit(bc, I);
}

void x64bytecode_emit_push(X64Bytecode *restrict bc, X64Operand A) {
  assert(bc != NULL);
  X64Instruction I = x64inst_A(X64OPC_PUSH, A);
  x64bytecode_emit(bc, I);
}

void x64bytecode_emit_pop(X64Bytecode *restrict bc, X64Operand A) {
  assert(bc != NULL);
  X64Instruction I = x64inst_A(X64OPC_POP, A);
  x64bytecode_emit(bc, I);
}

void x64bytecode_emit_mov(X64Bytecode *restrict bc, X64Operand A,
                          X64Operand B) {
  assert(bc != NULL);
  X64Instruction I = x64inst_AB(X64OPC_MOV, A, B);
  x64bytecode_emit(bc, I);
}

void x64bytecode_emit_add(X64Bytecode *restrict bc, X64Operand A,
                          X64Operand B) {
  assert(bc != NULL);
  X64Instruction I = x64inst_AB(X64OPC_ADD, A, B);
  x64bytecode_emit(bc, I);
}

void x64bytecode_emit_sub(X64Bytecode *restrict bc, X64Operand A,
                          X64Operand B) {
  assert(bc != NULL);
  X64Instruction I = x64inst_AB(X64OPC_SUB, A, B);
  x64bytecode_emit(bc, I);
}

void x64bytecode_emit_imul(X64Bytecode *restrict bc, X64Operand A) {
  assert(bc != NULL);
  X64Instruction I = x64inst_A(X64OPC_IMUL, A);
  x64bytecode_emit(bc, I);
}

void x64bytecode_emit_idiv(X64Bytecode *restrict bc, X64Operand A) {
  assert(bc != NULL);
  X64Instruction I = x64inst_A(X64OPC_IDIV, A);
  x64bytecode_emit(bc, I);
}