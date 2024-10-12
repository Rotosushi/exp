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
#include "imr/instruction.h"
#include "adt/string.h"
#include "env/context.h"

static Instruction instruction_B(Opcode opcode, Operand B) {
  Instruction I = {
      .opcode = opcode, .format = IFMT_B, .B_format = B.format, .B = B.value};
  return I;
}

static Instruction instruction_AB(Opcode opcode, Operand A, Operand B) {
  Instruction I = {.opcode   = opcode,
                   .format   = IFMT_AB,
                   .A_format = A.format,
                   .A        = A.value,
                   .B_format = B.format,
                   .B        = B.value};
  return I;
}

static Instruction
instruction_ABC(Opcode opcode, Operand A, Operand B, Operand C) {
  Instruction I = {.opcode   = opcode,
                   .format   = IFMT_ABC,
                   .A_format = A.format,
                   .A        = A.value,
                   .B_format = B.format,
                   .B        = B.value,
                   .C_format = C.format,
                   .C        = C.value};
  return I;
}

Instruction instruction_ret(Operand result) {
  return instruction_B(OPC_RET, result);
}

Instruction instruction_call(Operand dst, Operand label, Operand args) {
  return instruction_ABC(OPC_CALL, dst, label, args);
}

Instruction instruction_lea(Operand dst, Operand src) {
  return instruction_AB(OPC_LEA, dst, src);
}

Instruction instruction_dot(Operand dst, Operand src, Operand index) {
  return instruction_ABC(OPC_DOT, dst, src, index);
}

Instruction instruction_move(Operand dst, Operand src) {
  return instruction_AB(OPC_MOVE, dst, src);
}

Instruction instruction_neg(Operand dst, Operand src) {
  return instruction_AB(OPC_NEG, dst, src);
}

Instruction instruction_add(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_ADD, dst, left, right);
}

Instruction instruction_sub(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_SUB, dst, left, right);
}

Instruction instruction_mul(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_MUL, dst, left, right);
}

Instruction instruction_div(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_DIV, dst, left, right);
}

Instruction instruction_mod(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_MOD, dst, left, right);
}

static void print_B(StringView mnemonic,
                    Instruction I,
                    String *restrict out,
                    Context *restrict context) {
  string_append(out, mnemonic);
  string_append(out, SV(" "));
  print_operand(I.B_format, I.B, out, context);
}

static void print_AB(StringView mnemonic,
                     Instruction I,
                     String *restrict out,
                     Context *restrict context) {
  string_append(out, mnemonic);
  string_append(out, SV(" "));
  print_operand(I.A_format, I.A, out, context);
  string_append(out, SV(", "));
  print_operand(I.B_format, I.B, out, context);
}

static void print_ABC(StringView mnemonic,
                      Instruction I,
                      String *restrict out,
                      Context *restrict context) {
  string_append(out, mnemonic);
  string_append(out, SV(" "));
  print_operand(I.A_format, I.A, out, context);
  string_append(out, SV(", "));
  print_operand(I.B_format, I.B, out, context);
  string_append(out, SV(", "));
  print_operand(I.C_format, I.C, out, context);
}

// "ret <B>"
static void
print_ret(Instruction I, String *restrict out, Context *restrict context) {
  print_B(SV("ret"), I, out, context);
}

// "call SSA[<A>], GlobalSymbols[GlobalLabels[B]](Calls[C])"
static void
print_call(Instruction I, String *restrict out, Context *restrict context) {
  print_ABC(SV("call"), I, out, context);
}

// "dot SSA[<A>], <B>, <C>"
static void
print_dot(Instruction I, String *restrict out, Context *restrict context) {
  print_ABC(SV("dot"), I, out, context);
}

// "move SSA[<A>], <B>"
static void
print_move(Instruction I, String *restrict out, Context *restrict context) {
  print_AB(SV("move"), I, out, context);
}

// "neg SSA[<A>], <B>"
static void
print_neg(Instruction I, String *restrict out, Context *restrict context) {
  print_AB(SV("neg"), I, out, context);
}

// "add SSA[<A>], <B>, <C>"
static void
print_add(Instruction I, String *restrict out, Context *restrict context) {
  print_ABC(SV("add"), I, out, context);
}

// "sub SSA[<A>], <B>, <C>"
static void
print_sub(Instruction I, String *restrict out, Context *restrict context) {
  print_ABC(SV("sub"), I, out, context);
}

// "mul SSA[<A>], <B>, <C>"
static void
print_mul(Instruction I, String *restrict out, Context *restrict context) {
  print_ABC(SV("mul"), I, out, context);
}

// "div SSA[<A>], <B>, <C>"
static void
print_div(Instruction I, String *restrict out, Context *restrict context) {
  print_ABC(SV("div"), I, out, context);
}

// "mod SSA[<A>], <B>, <C>"
static void
print_mod(Instruction I, String *restrict out, Context *restrict context) {
  print_ABC(SV("mod"), I, out, context);
}

void instruction_print(Instruction I,
                       String *restrict out,
                       Context *restrict context) {
  switch (I.opcode) {
  case OPC_RET:  print_ret(I, out, context); break;
  case OPC_CALL: print_call(I, out, context); break;
  case OPC_DOT:  print_dot(I, out, context); break;
  case OPC_MOVE: print_move(I, out, context); break;
  case OPC_NEG:  print_neg(I, out, context); break;
  case OPC_ADD:  print_add(I, out, context); break;
  case OPC_SUB:  print_sub(I, out, context); break;
  case OPC_MUL:  print_mul(I, out, context); break;
  case OPC_DIV:  print_div(I, out, context); break;
  case OPC_MOD:  print_mod(I, out, context); break;

  default: unreachable();
  }
}
