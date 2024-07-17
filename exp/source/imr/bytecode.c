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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "imr/bytecode.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/io.h"
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
  deallocate(bytecode->buffer);
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
  if (bytecode_full(bytecode)) { bytecode_grow(bytecode); }

  bytecode->buffer[bytecode->length] = I;
  bytecode->length += 1;
}

// B -- L[R] = B,    <return>
// B -- L[R] = C[B], <return>
// B -- L[R] = L[B], <return>
void bytecode_emit_return(Bytecode *restrict bc, Operand B) {
  assert(bc != NULL);
  bytecode_emit_instruction(bc, instruction_B(OPC_RET, B));
}

void bytecode_emit_call(Bytecode *restrict bc,
                        Operand A,
                        Operand B,
                        Operand C) {
  assert(bc != NULL);
  bytecode_emit_instruction(bc, instruction_ABC(OPC_CALL, A, B, C));
}

// AB -- L[A] = B
// AB -- L[A] = C[B]
// AB -- L[A] = L[B]
void bytecode_emit_load(Bytecode *restrict bc, Operand A, Operand B) {
  assert(bc != NULL);
  bytecode_emit_instruction(bc, instruction_AB(OPC_LOAD, A, B));
}

// AB  -- L[A] = -(B)
// AB  -- L[A] = -(C[B])
// AB  -- L[A] = -(L[B])
void bytecode_emit_neg(Bytecode *restrict bc, Operand A, Operand B) {
  assert(bc != NULL);
  bytecode_emit_instruction(bc, instruction_AB(OPC_NEG, A, B));
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
  bytecode_emit_instruction(bc, instruction_ABC(OPC_ADD, A, B, C));
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
  bytecode_emit_instruction(bc, instruction_ABC(OPC_SUB, A, B, C));
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
  bytecode_emit_instruction(bc, instruction_ABC(OPC_MUL, A, B, C));
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
  bytecode_emit_instruction(bc, instruction_ABC(OPC_DIV, A, B, C));
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
  bytecode_emit_instruction(bc, instruction_ABC(OPC_MOD, A, B, C));
}

static Operand operand_A(Instruction I) {
  Operand operand = {.format = OPRFMT_SSA, .common = I.A};
  return operand;
}

static Operand operand_B(Instruction I) {
  Operand operand = {.format = I.Bfmt, .common = I.B};
  return operand;
}

static Operand operand_C(Instruction I) {
  Operand operand = {.format = I.Cfmt, .common = I.C};
  return operand;
}

static void
print_B(char const *restrict inst, Instruction I, FILE *restrict file) {
  file_write(inst, file);
  file_write(" ", file);
  print_operand(operand_B(I), file);
}

static void
print_AB(char const *restrict inst, Instruction I, FILE *restrict file) {
  file_write(inst, file);
  file_write(" ", file);
  print_operand(operand_A(I), file);
  file_write(", ", file);
  print_operand(operand_B(I), file);
}

static void
print_ABC(char const *restrict inst, Instruction I, FILE *restrict file) {
  file_write(inst, file);
  file_write(" ", file);
  print_operand(operand_A(I), file);
  file_write(", ", file);
  print_operand(operand_B(I), file);
  file_write(", ", file);
  print_operand(operand_C(I), file);
}

// "load L[<A>], <B>"
static void print_load(Instruction I, FILE *restrict file) {
  print_AB("load", I, file);
}

// "neg L[<A>], <B>"
static void print_neg(Instruction I, FILE *restrict file) {
  print_AB("neg", I, file);
}

// "add L[<A>], <B>, <C>"
static void print_add(Instruction I, FILE *restrict file) {
  print_ABC("add", I, file);
}

// "sub L[<A>], <B>, <C>"
static void print_sub(Instruction I, FILE *restrict file) {
  print_ABC("sub", I, file);
}

// "mul L[<A>], <B>, <C>"
static void print_mul(Instruction I, FILE *restrict file) {
  print_ABC("mul", I, file);
}

// "div L[<A>], <B>, <C>"
static void print_div(Instruction I, FILE *restrict file) {
  print_ABC("div", I, file);
}

// "mod L[<A>], <B>, <C>"
static void print_mod(Instruction I, FILE *restrict file) {
  print_ABC("mod", I, file);
}

// "ret <B>"
static void print_ret(Instruction I, FILE *restrict file) {
  print_B("ret", I, file);
}

static void print_call(Instruction I, FILE *restrict file) {
  print_ABC("call", I, file);
}

static void print_instruction(Instruction I, FILE *restrict file) {
  switch (I.opcode) {
  case OPC_RET:  print_ret(I, file); break;
  case OPC_CALL: print_call(I, file); break;
  case OPC_LOAD: print_load(I, file); break;
  case OPC_NEG:  print_neg(I, file); break;
  case OPC_ADD:  print_add(I, file); break;
  case OPC_SUB:  print_sub(I, file); break;
  case OPC_MUL:  print_mul(I, file); break;
  case OPC_DIV:  print_div(I, file); break;
  case OPC_MOD:  print_mod(I, file); break;

  default: file_write("undefined", file); break;
  }
}

void print_bytecode(Bytecode const *restrict bc, FILE *restrict file) {
  // walk the entire buffer and print each instruction
  for (u64 i = 0; i < bc->length; ++i) {
    print_u64(i, file);
    file_write(": ", file);
    print_instruction(bc->buffer[i], file);
    file_write("\n", file);
  }
}
