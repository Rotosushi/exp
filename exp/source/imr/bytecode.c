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
#include <stddef.h>

#include "imr/bytecode.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/io.h"

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
  Growth g         = array_growth_u64(bytecode->capacity, sizeof(Instruction));
  bytecode->buffer = reallocate(bytecode->buffer, g.alloc_size);
  bytecode->capacity = g.new_capacity;
}

void bytecode_append(Bytecode *restrict bytecode, Instruction I) {
  if (bytecode_full(bytecode)) { bytecode_grow(bytecode); }

  bytecode->buffer[bytecode->length] = I;
  bytecode->length += 1;
}

static void
print_B(char const *restrict inst, Instruction I, FILE *restrict file) {
  file_write(inst, file);
  file_write(" ", file);
  print_operand(I.B, file);
}

static void
print_AB(char const *restrict inst, Instruction I, FILE *restrict file) {
  file_write(inst, file);
  file_write(" ", file);
  print_ssa(I.A, file);
  file_write(", ", file);
  print_operand(I.B, file);
}

static void
print_ABC(char const *restrict inst, Instruction I, FILE *restrict file) {
  file_write(inst, file);
  file_write(" ", file);
  print_ssa(I.A, file);
  file_write(", ", file);
  print_operand(I.B, file);
  file_write(", ", file);
  print_operand(I.C, file);
}

// "ret <B>"
static void print_ret(Instruction I, FILE *restrict file) {
  print_B("ret", I, file);
}

// "call SSA[<A>], GlobalSymbols[GlobalLabels[B]](Calls[C])"
static void print_call(Instruction I, FILE *restrict file) {
  print_ABC("call", I, file);
}

// "dot SSA[<A>], <B>, <C>"
static void print_dot(Instruction I, FILE *restrict file) {
  print_ABC("dot", I, file);
}

// "load SSA[<A>], <B>"
static void print_load(Instruction I, FILE *restrict file) {
  print_AB("load", I, file);
}

// "neg SSA[<A>], <B>"
static void print_neg(Instruction I, FILE *restrict file) {
  print_AB("neg", I, file);
}

// "add SSA[<A>], <B>, <C>"
static void print_add(Instruction I, FILE *restrict file) {
  print_ABC("add", I, file);
}

// "sub SSA[<A>], <B>, <C>"
static void print_sub(Instruction I, FILE *restrict file) {
  print_ABC("sub", I, file);
}

// "mul SSA[<A>], <B>, <C>"
static void print_mul(Instruction I, FILE *restrict file) {
  print_ABC("mul", I, file);
}

// "div SSA[<A>], <B>, <C>"
static void print_div(Instruction I, FILE *restrict file) {
  print_ABC("div", I, file);
}

// "mod SSA[<A>], <B>, <C>"
static void print_mod(Instruction I, FILE *restrict file) {
  print_ABC("mod", I, file);
}

static void print_instruction(Instruction I, FILE *restrict file) {
  switch (I.opcode) {
  case OPC_RET:  print_ret(I, file); break;
  case OPC_CALL: print_call(I, file); break;
  case OPC_DOT:  print_dot(I, file); break;
  case OPC_LOAD: print_load(I, file); break;
  case OPC_NEG:  print_neg(I, file); break;
  case OPC_ADD:  print_add(I, file); break;
  case OPC_SUB:  print_sub(I, file); break;
  case OPC_MUL:  print_mul(I, file); break;
  case OPC_DIV:  print_div(I, file); break;
  case OPC_MOD:  print_mod(I, file); break;

  default: unreachable();
  }
}

void print_bytecode(Bytecode const *restrict bc, FILE *restrict file) {
  // walk the entire buffer and print each instruction
  for (u64 i = 0; i < bc->length; ++i) {
    file_write_u64(i, file);
    file_write(": ", file);
    print_instruction(bc->buffer[i], file);
    file_write("\n", file);
  }
}
