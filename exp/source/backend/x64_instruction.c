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

#include "backend/x64_instruction.h"

X64Operand x64opr_gpr(u16 gpr) {
  X64Operand opr = {.format = X64OPRFMT_GPR, .common = gpr};
  return opr;
}

X64Operand x64opr_stack(u16 offset) {
  X64Operand opr = {.format = X64OPRFMT_STACK, .common = offset};
  return opr;
}

X64Operand x64opr_alloc(X64Allocation *alloc) {
  switch (alloc->kind) {
  case ALLOC_GPR: {
    return x64opr_gpr(alloc->gpr);
  }

  case ALLOC_STACK: {
    return x64opr_stack(alloc->offset);
  }

  default: unreachable();
  }
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
  X64Instruction I = {.opcode = opcode, .Afmt = A.format, .A = A.common};
  return I;
}

X64Instruction x64inst_AB(X64Opcode opcode, X64Operand A, X64Operand B) {
  X64Instruction I = {.opcode = opcode,
                      .Afmt   = A.format,
                      .A      = A.common,
                      .Bfmt   = B.format,
                      .B      = B.common};
  return I;
}

void x64opr_print(X64OperandFormat fmt,
                  u16 common,
                  String *restrict buffer,
                  Context *restrict context) {
  switch (fmt) {
  case X64OPRFMT_GPR: {
    string_append(buffer, SV("%"));
    string_append(buffer, gpr_to_sv(common));
    break;
  }

  case X64OPRFMT_STACK: {
    string_append(buffer, SV("-"));
    string_append_u64(buffer, common);
    string_append(buffer, SV("(%rbp)"));
    break;
  }

  case X64OPRFMT_CONSTANT: {
    Value *constant = context_constants_at(context, common);
    assert(constant->kind == VALUEKIND_I64);
    string_append(buffer, SV("$"));
    string_append_i64(buffer, constant->integer);
    break;
  }

  case X64OPRFMT_IMMEDIATE: {
    string_append(buffer, SV("$"));
    string_append_i64(buffer, common);
    break;
  }

  default: unreachable();
  }
}

void x64inst_emit(X64Instruction I,
                  String *restrict buffer,
                  Context *restrict context) {
  switch (I.opcode) {
  case X64OPC_RET: {
    string_append(buffer, SV("ret"));
    break;
  }

  case X64OPC_PUSH: {
    string_append(buffer, SV("push "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_POP: {
    string_append(buffer, SV("pop "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_MOV: {
    string_append(buffer, SV("mov "));
    x64opr_print(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_NEG: {
    string_append(buffer, SV("neg "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_ADD: {
    string_append(buffer, SV("add "));
    x64opr_print(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_SUB: {
    string_append(buffer, SV("sub "));
    x64opr_print(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_IMUL: {
    string_append(buffer, SV("imul "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_IDIV: {
    string_append(buffer, SV("idiv "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  default: unreachable();
  }
}