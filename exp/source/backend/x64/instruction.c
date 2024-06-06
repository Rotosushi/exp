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

#include "backend/x64/instruction.h"

x64_Instruction x64_inst(x64_Opcode opcode) {
  x64_Instruction I = {.opcode = opcode};
  return I;
}

x64_Instruction x64_inst_A(x64_Opcode opcode, x64_Operand A) {
  x64_Instruction I = {.opcode = opcode, .Afmt = A.format, .A = A.common};
  return I;
}

x64_Instruction x64_inst_AB(x64_Opcode opcode, x64_Operand A, x64_Operand B) {
  x64_Instruction I = {.opcode = opcode,
                       .Afmt   = A.format,
                       .A      = A.common,
                       .Bfmt   = B.format,
                       .B      = B.common};
  return I;
}

void x64opr_print(x64_OperandFormat fmt,
                  u16 common,
                  String *restrict buffer,
                  Context *restrict context) {
  switch (fmt) {
  case X64OPRFMT_GPR: {
    string_append(buffer, SV("%"));
    string_append(buffer, x64_gpr_to_sv(common));
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

void x64_inst_emit(x64_Instruction I,
                   String *restrict buffer,
                   Context *restrict context) {
  switch (I.opcode) {
  case X64OPC_RET: {
    string_append(buffer, SV("ret"));
    break;
  }

  case X64OPC_PUSH: {
    string_append(buffer, SV("pushq "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_POP: {
    string_append(buffer, SV("popq "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_MOV: {
    string_append(buffer, SV("movq "));
    x64opr_print(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_NEG: {
    string_append(buffer, SV("negq "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_ADD: {
    string_append(buffer, SV("addq "));
    x64opr_print(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_SUB: {
    string_append(buffer, SV("subq "));
    x64opr_print(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_IMUL: {
    string_append(buffer, SV("imulq "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_IDIV: {
    string_append(buffer, SV("idivq "));
    x64opr_print(I.Afmt, I.A, buffer, context);
    break;
  }

  default: unreachable();
  }
}