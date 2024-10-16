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

static Instruction instruction_B(Opcode opcode, Operand B) {
  Instruction I = {.opcode = opcode, .format = IFMT_B, .B = B};
  return I;
}

static Instruction instruction_AB(Opcode opcode, u64 A, Operand B) {
  Instruction I = {.opcode = opcode, .format = IFMT_AB, .A = A, .B = B};
  return I;
}

static Instruction instruction_ABC(Opcode opcode, u64 A, Operand B, Operand C) {
  Instruction I = {
      .opcode = opcode, .format = IFMT_ABC, .A = A, .B = B, .C = C};
  return I;
}

Instruction instruction_ret(Operand result) {
  return instruction_B(OPC_RET, result);
}

Instruction instruction_call(Operand dst, Operand label, Operand args) {
  return instruction_ABC(OPC_CALL, dst.ssa, label, args);
}

Instruction instruction_dot(Operand dst, Operand src, Operand index) {
  return instruction_ABC(OPC_DOT, dst.ssa, src, index);
}

Instruction instruction_load(Operand dst, Operand src) {
  return instruction_AB(OPC_LOAD, dst.ssa, src);
}

Instruction instruction_neg(Operand dst, Operand src) {
  return instruction_AB(OPC_NEG, dst.ssa, src);
}

Instruction instruction_add(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_ADD, dst.ssa, left, right);
}

Instruction instruction_sub(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_SUB, dst.ssa, left, right);
}

Instruction instruction_mul(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_MUL, dst.ssa, left, right);
}

Instruction instruction_div(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_DIV, dst.ssa, left, right);
}

Instruction instruction_mod(Operand dst, Operand left, Operand right) {
  return instruction_ABC(OPC_MOD, dst.ssa, left, right);
}
