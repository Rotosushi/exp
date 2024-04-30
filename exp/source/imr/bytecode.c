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
#include "utility/array_growth.h"
#include "utility/numeric_conversions.h"
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
  Growth g         = array_growth_u16(bytecode->capacity, sizeof(Instruction));
  bytecode->buffer = reallocate(bytecode->buffer, g.alloc_size);
  bytecode->capacity = (u16)g.new_capacity;
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
  INST_SET_FORMAT(I, IFMT_B);                                                \
  INST_SET_B_FORMAT(I, B.format);                                              \
  INST_SET_B(I, B.common)

#define FORMAT_AB(I, OP, A, B)                                                 \
  INST_SET_OP(I, OP);                                                          \
  INST_SET_FORMAT(I, IFMT_AB);                                               \
  INST_SET_A(I, A.common);                                                     \
  INST_SET_B_FORMAT(I, B.format);                                              \
  INST_SET_B(I, B.common)

#define FORMAT_ABC(I, OP, A, B, C)                                             \
  INST_SET_OP(I, OP);                                                          \
  INST_SET_FORMAT(I, IFMT_ABC);                                              \
  INST_SET_A(I, A.common);                                                     \
  INST_SET_B_FORMAT(I, B.format);                                              \
  INST_SET_B(I, B.common);                                                     \
  INST_SET_C_FORMAT(I, C.format);                                              \
  INST_SET_C(I, C.common)

// AB -- L[A] = B
// AB -- L[A] = C[B]
// AB -- L[A] = L[B]
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
  FORMAT_B(I, OPC_RET, B);

  bytecode_emit_instruction(bc, I);
}

static void print_local(u16 v, FILE *restrict file) {
  file_write("L[", file);
  print_u64(v, RADIX_DECIMAL, file);
  file_write("]", file);
}

static void print_constant(u16 v, FILE *restrict file) {
  file_write("C[", file);
  print_u64(v, RADIX_DECIMAL, file);
  file_write("]", file);
}

static void print_immediate(u16 v, FILE *restrict file) {
  print_i64((i16)v, RADIX_DECIMAL, file);
}

static void print_operand(OperandFormat format, u16 value,
                          FILE *restrict file) {
  switch (format) {
  case OPRFMT_SSA:
    print_local(value, file);
    break;

  case OPRFMT_CONSTANT:
    print_constant(value, file);
    break;

  case OPRFMT_IMMEDIATE:
    print_immediate(value, file);
    break;

  default:
    file_write("undefined", file);
  }
}

static void print_operand_A(Instruction I, FILE *restrict file) {
  u16 v = INST_A(I);
  print_operand(OPRFMT_SSA, v, file);
}

static void print_operand_B(Instruction I, FILE *restrict file) {
  OperandFormat o = INST_B_FORMAT(I);
  u16 v           = INST_B(I);
  print_operand(o, v, file);
}

static void print_operand_C(Instruction I, FILE *restrict file) {
  OperandFormat o = INST_C_FORMAT(I);
  u16 v           = INST_C(I);
  print_operand(o, v, file);
}

// "move L[<A>], <B>"
static void print_move(Instruction I, FILE *restrict file) {
  file_write("move ", file);
  print_operand_A(I, file);
  file_write(", ", file);
  print_operand_B(I, file);
}

// "neg L[<A>], <B>"
static void print_neg(Instruction I, FILE *restrict file) {
  file_write("neg ", file);
  print_operand_A(I, file);
  file_write(", ", file);
  print_operand_B(I, file);
}

// "add L[<A>], <B>, <C>"
static void print_add(Instruction I, FILE *restrict file) {
  file_write("add ", file);
  print_operand_A(I, file);
  file_write(", ", file);
  print_operand_B(I, file);
  file_write(", ", file);
  print_operand_C(I, file);
}

// "sub L[<A>], <B>, <C>"
static void print_sub(Instruction I, FILE *restrict file) {
  file_write("sub ", file);
  print_operand_A(I, file);
  file_write(", ", file);
  print_operand_B(I, file);
  file_write(", ", file);
  print_operand_C(I, file);
}

// "mul L[<A>], <B>, <C>"
static void print_mul(Instruction I, FILE *restrict file) {
  file_write("mul ", file);
  print_operand_A(I, file);
  file_write(", ", file);
  print_operand_B(I, file);
  file_write(", ", file);
  print_operand_C(I, file);
}

// "div L[<A>], <B>, <C>"
static void print_div(Instruction I, FILE *restrict file) {
  file_write("div ", file);
  print_operand_A(I, file);
  file_write(", ", file);
  print_operand_B(I, file);
  file_write(", ", file);
  print_operand_C(I, file);
}

// "mod L[<A>], <B>, <C>"
static void print_mod(Instruction I, FILE *restrict file) {
  file_write("mod ", file);
  print_operand_A(I, file);
  file_write(", ", file);
  print_operand_B(I, file);
  file_write(", ", file);
  print_operand_C(I, file);
}

// "ret <B>"
static void print_ret(Instruction I, FILE *restrict file) {
  file_write("ret ", file);
  print_operand_B(I, file);
}

static void print_instruction(Instruction I, FILE *restrict file) {
  switch (INST_OP(I)) {
  case OPC_MOVE:
    print_move(I, file);
    break;

  case OPC_NEG:
    print_neg(I, file);
    break;

  case OPC_ADD:
    print_add(I, file);
    break;

  case OPC_SUB:
    print_sub(I, file);
    break;

  case OPC_MUL:
    print_mul(I, file);
    break;

  case OPC_DIV:
    print_div(I, file);
    break;

  case OPC_MOD:
    print_mod(I, file);
    break;

  case OPC_RET:
    print_ret(I, file);
    break;

  default:
    file_write("undefined", file);
    break;
  }
}

void print_bytecode(Bytecode const *restrict bc, FILE *restrict file) {
  // walk the entire buffer and print each instruction
  for (u64 i = 0; i < bc->length; ++i) {
    print_u64(i, RADIX_DECIMAL, file);
    file_write(": ", file);
    print_instruction(bc->buffer[i], file);
    file_write("\n", file);
  }
}