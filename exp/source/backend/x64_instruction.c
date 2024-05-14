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
#include "backend/x64_instruction.h"

X64Operand x64opr_gpr(u16 gpr) {
  X64Operand opr = {.format = X64OPRFMT_GPR, .common = gpr};
  return opr;
}

X64Operand x64opr_stack(u16 offset) {
  X64Operand opr = {.format = X64OPRFMT_STACK, .common = offset};
  return opr;
}

X64Operand x64opr_constant(u16 idx) {
  X64Operand opr = {.format = X64OPRFMT_CONSTANT, .common = idx};
  return opr;
}

X64Operand x64opr_immediate(u16 n) {
  X64Operand opr = {.format = X64OPRFMT_IMMEDIATE, .common = n};
  return opr;
}

X64Instruction x64inst(X64Opcode opcode) {
  X64Instruction I = {.opcode = opcode};
  return I;
}

X64Instruction x64inst_A(X64Opcode opcode, X64Operand A) {
  X64Instruction I = {.opcode = opcode, .A_fmt = A.format, .A = A.common};
  return I;
}

X64Instruction x64inst_AB(X64Opcode opcode, X64Operand A, X64Operand B) {
  X64Instruction I = {.opcode = opcode,
                      .A_fmt  = A.format,
                      .B_fmt  = B.format,
                      .A      = A.common,
                      .B      = B.common};
  return I;
}