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

static void x64_emit_mnemonic(StringView mnemonic,
                              [[maybe_unused]] x64_Instruction I,
                              String *restrict buffer,
                              [[maybe_unused]] Context *restrict context) {
  string_append(buffer, mnemonic);
  // if either operand is a register then it is a 64 bit GPR, so we
  // know we need the 'q' suffix. because, as a simplification, we
  // only support the 64 bit GPRs.
  //
  // as a simplification, we always allocate a 64 bit word for each
  // type we currently support, so we can safely always emit the 'q'
  // suffix to handle any type residing on the stack,
  // (in order to properly handle this we need to get the size
  // of the type that the operand is representing.)
  //
  // if the operand is an immediate then we know it must be either
  // a single byte 'b' or a single word 'w'. whereas a constant operand
  // can need the suffix 'b', 'w', 'l', or 'q'.
  //
  // however, relying on the above simplification, we always load/store
  // a quad word, so we can always emit the 'q' suffix.
  string_append(buffer, SV("q "));
}

static void x64_emit_opr(x64_OperandFormat fmt,
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
    x64_emit_mnemonic(SV("push"), I, buffer, context);
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_POP: {
    x64_emit_mnemonic(SV("pop"), I, buffer, context);
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_MOV: {
    x64_emit_mnemonic(SV("mov"), I, buffer, context);
    x64_emit_opr(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_NEG: {
    x64_emit_mnemonic(SV("neg"), I, buffer, context);
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_ADD: {
    x64_emit_mnemonic(SV("add"), I, buffer, context);
    x64_emit_opr(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_SUB: {
    x64_emit_mnemonic(SV("sub"), I, buffer, context);
    x64_emit_opr(I.Bfmt, I.B, buffer, context);
    string_append(buffer, SV(", "));
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_IMUL: {
    x64_emit_mnemonic(SV("imul"), I, buffer, context);
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  case X64OPC_IDIV: {
    x64_emit_mnemonic(SV("idiv"), I, buffer, context);
    x64_emit_opr(I.Afmt, I.A, buffer, context);
    break;
  }

  default: unreachable();
  }
}