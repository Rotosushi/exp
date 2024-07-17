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

x64_Instruction x64_instruction(x64_Opcode opcode) {
  x64_Instruction I = {.opcode = opcode};
  return I;
}

x64_Instruction x64_instruction_A(x64_Opcode opcode, x64_Operand A) {
  x64_Instruction I = {.opcode = opcode, .Afmt = A.format};

  if (A.format == X64OPRFMT_STACK) {
    I.Aoffset = A.offset;
  } else {
    I.Acommon = A.common;
  }

  return I;
}

x64_Instruction
x64_instruction_AB(x64_Opcode opcode, x64_Operand A, x64_Operand B) {
  x64_Instruction I = {.opcode = opcode, .Afmt = A.format, .Bfmt = B.format};

  if (A.format == X64OPRFMT_STACK) {
    I.Aoffset = A.offset;
  } else {
    I.Acommon = A.common;
  }

  if (B.format == X64OPRFMT_STACK) {
    I.Boffset = B.offset;
  } else {
    I.Bcommon = B.common;
  }

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
  // type we currently support.
  //
  // #TODO:
  // we need to get the size of the type that the operand is representing
  // and choose the correct mnemonic suffix accordingly.
  // 'b' -> u8
  // 'w' -> u16
  // 'l' -> u32
  // 'q' -> u64
  //
  // additionally
  // 's' -> f32
  // 'l' -> f64
  // 't' -> f80
  //
  // however, relying on the above simplifications, we always load/store
  // a quad word, so we can always emit the 'q' suffix.
  string_append(buffer, SV("q\t"));
}

static void x64_emit_operand(x64_Operand O,
                             String *restrict buffer,
                             Context *restrict context) {
  switch (O.format) {
  case X64OPRFMT_GPR: {
    string_append(buffer, SV("%"));
    string_append(buffer, x64_gpr_to_sv((x64_GPR)O.common));
    break;
  }

  case X64OPRFMT_STACK: {
    string_append_i64(buffer, O.offset);
    string_append(buffer, SV("(%rbp)"));
    break;
  }

  case X64OPRFMT_CONSTANT: {
    Value *constant = context_constants_at(context, O.common);
    assert(constant->kind == VALUEKIND_I64);
    string_append(buffer, SV("$"));
    string_append_i64(buffer, constant->integer_64);
    break;
  }

  case X64OPRFMT_IMMEDIATE: {
    string_append(buffer, SV("$"));
    string_append_i64(buffer, O.common);
    break;
  }

  case X64OPRFMT_LABEL: {
    StringView name = context_global_labels_at(context, O.common);
    string_append(buffer, name);
    break;
  }

  default: unreachable();
  }
}

static x64_Operand operand_A(x64_Instruction I) {
  if (I.Afmt == X64OPRFMT_STACK) {
    return (x64_Operand){.format = I.Afmt, .offset = I.Aoffset};
  }

  return (x64_Operand){.format = I.Afmt, .common = I.Acommon};
}

static x64_Operand operand_B(x64_Instruction I) {
  if (I.Bfmt == X64OPRFMT_STACK) {
    return (x64_Operand){.format = I.Bfmt, .offset = I.Boffset};
  }

  return (x64_Operand){.format = I.Bfmt, .common = I.Bcommon};
}

void x64_instruction_emit(x64_Instruction I,
                          String *restrict buffer,
                          Context *restrict context) {
  switch (I.opcode) {
  case X64OPC_RET: {
    string_append(buffer, SV("ret"));
    break;
  }

  case X64OPC_CALL: {
    string_append(buffer, SV("call\t"));
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_PUSH: {
    x64_emit_mnemonic(SV("push"), I, buffer, context);
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_POP: {
    x64_emit_mnemonic(SV("pop"), I, buffer, context);
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_MOV: {
    x64_emit_mnemonic(SV("mov"), I, buffer, context);
    x64_emit_operand(operand_B(I), buffer, context);
    string_append(buffer, SV(", "));
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_NEG: {
    x64_emit_mnemonic(SV("neg"), I, buffer, context);
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_ADD: {
    x64_emit_mnemonic(SV("add"), I, buffer, context);
    x64_emit_operand(operand_B(I), buffer, context);
    string_append(buffer, SV(", "));
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_SUB: {
    x64_emit_mnemonic(SV("sub"), I, buffer, context);
    x64_emit_operand(operand_B(I), buffer, context);
    string_append(buffer, SV(", "));
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_IMUL: {
    x64_emit_mnemonic(SV("imul"), I, buffer, context);
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  case X64OPC_IDIV: {
    x64_emit_mnemonic(SV("idiv"), I, buffer, context);
    x64_emit_operand(operand_A(I), buffer, context);
    break;
  }

  default: unreachable();
  }
}