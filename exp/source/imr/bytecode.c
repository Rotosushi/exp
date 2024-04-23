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
#include "utility/alloc.h"
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

  u64 alloc_size;
  if (__builtin_mul_overflow(new_capacity, sizeof(Instruction), &alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  bytecode->buffer   = reallocate(bytecode->buffer, new_capacity);
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

#define FORMAT_B(I, OP, B)                                                     \
  INST_SET_OP(I, OP);                                                          \
  INST_SET_FORMAT(I, FORMAT_B);                                                \
  INST_SET_B_FORMAT(I, B.format);                                              \
  INST_SET_B(I, B.common)

#define FORMAT_AB(I, OP, A, B)                                                 \
  INST_SET_OP(I, OP);                                                          \
  INST_SET_FORMAT(I, FORMAT_AB);                                               \
  INST_SET_A(I, A.common);                                                     \
  INST_SET_B_FORMAT(I, B.format);                                              \
  INST_SET_B(I, B.common)

#define FORMAT_ABC(I, OP, A, B, C)                                             \
  INST_SET_OP(I, OP);                                                          \
  INST_SET_FORMAT(I, FORMAT_ABC);                                              \
  INST_SET_A(I, A.common);                                                     \
  INST_SET_B_FORMAT(I, B.format);                                              \
  INST_SET_B(I, B.common);                                                     \
  INST_SET_C_FORMAT(I, C.format);                                              \
  INST_SET_C(I, C.common)

//  AB  -- L[A] = B
//  AB  -- L[A] = C[B]
//  AB  -- L[A] = L[B]
void bytecode_emit_move(Bytecode *restrict bc, Operand A, Operand B) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_AB(I, OPC_MOVE, A, B);

  bytecode_emit_instruction(bc, I);
}
// AB  -- L[A] = -(B)
// AB  -- L[A] = -(C[B])
// AB  -- L[A] = -(L[B])
void bytecode_emit_neg(Bytecode *restrict bc, Operand A, Operand B) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_AB(I, OPC_NEG, A, B);

  bytecode_emit_instruction(bc, I);
}

// ABC -- L[A] = L[B] + L[C]
// ABC -- L[A] = L[B] + C[C]
// ABC -- L[A] = L[B] + C
// ABC -- L[A] = C[B] + L[C]
// ABC -- L[A] = C[B] + C[C]
// ABC -- L[A] = C[B] + C
// ABC -- L[A] = B    + L[C]
// ABC -- L[A] = B    + C[C]
// ABC -- L[A] = B    + C
void bytecode_emit_add(Bytecode *restrict bc, Operand A, Operand B, Operand C) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_ABC(I, OPC_ADD, A, B, C);

  bytecode_emit_instruction(bc, I);
}

// ABC -- L[A] = L[B] - L[C]
// ABC -- L[A] = L[B] - C[C]
// ABC -- L[A] = L[B] - C
// ABC -- L[A] = C[B] - L[C]
// ABC -- L[A] = C[B] - C[C]
// ABC -- L[A] = C[B] - C
// ABC -- L[A] = B    - L[C]
// ABC -- L[A] = B    - C[C]
// ABC -- L[A] = B    - C
void bytecode_emit_sub(Bytecode *restrict bc, Operand A, Operand B, Operand C) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_ABC(I, OPC_SUB, A, B, C);

  bytecode_emit_instruction(bc, I);
}

// ABC -- L[A] = L[B] * L[C]
// ABC -- L[A] = L[B] * C[C]
// ABC -- L[A] = L[B] * C
// ABC -- L[A] = C[B] * L[C]
// ABC -- L[A] = C[B] * C[C]
// ABC -- L[A] = C[B] * C
// ABC -- L[A] = B    * L[C]
// ABC -- L[A] = B    * C[C]
// ABC -- L[A] = B    * C
void bytecode_emit_mul(Bytecode *restrict bc, Operand A, Operand B, Operand C) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_ABC(I, OPC_MUL, A, B, C);

  bytecode_emit_instruction(bc, I);
}

// ABC -- L[A] = L[B] / L[C]
// ABC -- L[A] = L[B] / C[C]
// ABC -- L[A] = L[B] / C
// ABC -- L[A] = C[B] / L[C]
// ABC -- L[A] = C[B] / C[C]
// ABC -- L[A] = C[B] / C
// ABC -- L[A] = B    / L[C]
// ABC -- L[A] = B    / C[C]
// ABC -- L[A] = B    / C
void bytecode_emit_div(Bytecode *restrict bc, Operand A, Operand B, Operand C) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_ABC(I, OPC_DIV, A, B, C);

  bytecode_emit_instruction(bc, I);
}

// ABC -- L[A] = L[B] % L[C]
// ABC -- L[A] = L[B] % C[C]
// ABC -- L[A] = L[B] % C
// ABC -- L[A] = C[B] % L[C]
// ABC -- L[A] = C[B] % C[C]
// ABC -- L[A] = C[B] % C
// ABC -- L[A] = B    % L[C]
// ABC -- L[A] = B    % C[C]
// ABC -- L[A] = B    % C
void bytecode_emit_mod(Bytecode *restrict bc, Operand A, Operand B, Operand C) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_ABC(I, OPC_MOD, A, B, C);

  bytecode_emit_instruction(bc, I);
}

// B -- L[R] = B,    <return>
// B -- L[R] = C[B], <return>
// B -- L[R] = L[B], <return>
void bytecode_emit_return(Bytecode *restrict bc, Operand B) {
  assert(bc != NULL);

  Instruction I = 0;
  FORMAT_B(I, OPC_RETURN, B);

  bytecode_emit_instruction(bc, I);
}