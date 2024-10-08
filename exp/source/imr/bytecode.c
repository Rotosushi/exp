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

#include "env/context.h"
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

static void print_B(char const *restrict inst,
                    Instruction I,
                    FILE *restrict file,
                    Context *restrict context) {
  file_write(inst, file);
  file_write(" ", file);
  print_operand(I.B, file, context);
}

static void print_AB(char const *restrict inst,
                     Instruction I,
                     FILE *restrict file,
                     Context *restrict context) {
  file_write(inst, file);
  file_write(" ", file);
  print_operand(I.A, file, context);
  file_write(", ", file);
  print_operand(I.B, file, context);
}

static void print_ABC(char const *restrict inst,
                      Instruction I,
                      FILE *restrict file,
                      Context *restrict context) {
  file_write(inst, file);
  file_write(" ", file);
  print_operand(I.A, file, context);
  file_write(", ", file);
  print_operand(I.B, file, context);
  file_write(", ", file);
  print_operand(I.C, file, context);
}

// "ret <B>"
static void
print_ret(Instruction I, FILE *restrict file, Context *restrict context) {
  print_B("ret", I, file, context);
}

// "call SSA[<A>], GlobalSymbols[GlobalLabels[B]](Calls[C])"
static void
print_call(Instruction I, FILE *restrict file, Context *restrict context) {
  print_ABC("call", I, file, context);
}

// "dot SSA[<A>], <B>, <C>"
static void
print_dot(Instruction I, FILE *restrict file, Context *restrict context) {
  print_ABC("dot", I, file, context);
}

// "move SSA[<A>], <B>"
static void
print_move(Instruction I, FILE *restrict file, Context *restrict context) {
  print_AB("move", I, file, context);
}

// "neg SSA[<A>], <B>"
static void
print_neg(Instruction I, FILE *restrict file, Context *restrict context) {
  print_AB("neg", I, file, context);
}

// "add SSA[<A>], <B>, <C>"
static void
print_add(Instruction I, FILE *restrict file, Context *restrict context) {
  print_ABC("add", I, file, context);
}

// "sub SSA[<A>], <B>, <C>"
static void
print_sub(Instruction I, FILE *restrict file, Context *restrict context) {
  print_ABC("sub", I, file, context);
}

// "mul SSA[<A>], <B>, <C>"
static void
print_mul(Instruction I, FILE *restrict file, Context *restrict context) {
  print_ABC("mul", I, file, context);
}

// "div SSA[<A>], <B>, <C>"
static void
print_div(Instruction I, FILE *restrict file, Context *restrict context) {
  print_ABC("div", I, file, context);
}

// "mod SSA[<A>], <B>, <C>"
static void
print_mod(Instruction I, FILE *restrict file, Context *restrict context) {
  print_ABC("mod", I, file, context);
}

static void print_instruction(Instruction I,
                              FILE *restrict file,
                              Context *restrict context) {
  switch (I.opcode) {
  case OPC_RET:  print_ret(I, file, context); break;
  case OPC_CALL: print_call(I, file, context); break;
  case OPC_DOT:  print_dot(I, file, context); break;
  case OPC_MOVE: print_move(I, file, context); break;
  case OPC_NEG:  print_neg(I, file, context); break;
  case OPC_ADD:  print_add(I, file, context); break;
  case OPC_SUB:  print_sub(I, file, context); break;
  case OPC_MUL:  print_mul(I, file, context); break;
  case OPC_DIV:  print_div(I, file, context); break;
  case OPC_MOD:  print_mod(I, file, context); break;

  default: unreachable();
  }
}

void print_bytecode(Bytecode const *restrict bc,
                    FILE *restrict file,
                    Context *restrict context) {
  // walk the entire buffer and print each instruction
  for (u64 i = 0; i < bc->length; ++i) {
    file_write("  ", file);
    file_write_u64(i, file);
    file_write(": ", file);
    print_instruction(bc->buffer[i], file, context);
    file_write("\n", file);
  }
}
