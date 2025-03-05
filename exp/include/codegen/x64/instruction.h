// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X64_INSTRUCTION_H
#define EXP_BACKEND_X64_INSTRUCTION_H

#include "codegen/x64/operand.h"
#include "env/context.h"
#include "utility/string.h"

/*
  We want to model the x64 instructions we generate
  in memory, so we can compare different instruction
  sequences and choose the most efficient one.

  this is for the future however. It is my hope that
  by doing this it will be possible to design an
  api for creating x64 instructions from bytecode
  instructions which will allow us to reduce the
  size and complexity of the codegen routines.
  because we will separate generating x64 instructions
  from emitting x64 instructions.

  --

  generally speaking x64 instructions use the opcode
  and the first operand together to determine the
  size of the operands.

  so we are simplifying this for now, just to represent
  what we need to. all operands are going to be 64 bit.

  #TODO emiting instructions needs to take into account the size of
  their operands, such that we can append the correct mnemonic
  suffix to the x64 assembly instruction.

*/

typedef enum x64_Opcode : u8 {
    X64_OPCODE_RETURN,
    X64_OPCODE_CALL,
    X64_OPCODE_PUSH,
    X64_OPCODE_POP,
    X64_OPCODE_MOV,
    X64_OPCODE_LEA,
    X64_OPCODE_NEG,
    X64_OPCODE_ADD,
    X64_OPCODE_SUB,
    X64_OPCODE_IMUL,
    X64_OPCODE_IDIV,
} x64_Opcode;

typedef struct x64_Instruction {
    x64_Opcode opcode;
    x64_Operand A;
    x64_Operand B;
} x64_Instruction;

x64_Instruction x64_ret();
x64_Instruction x64_call(x64_Operand label);
x64_Instruction x64_push(x64_Operand src);
x64_Instruction x64_pop(x64_Operand dst);
x64_Instruction x64_mov(x64_Operand dst, x64_Operand src);
x64_Instruction x64_lea(x64_Operand dst, x64_Operand src);

x64_Instruction x64_neg(x64_Operand dst);
x64_Instruction x64_add(x64_Operand dst, x64_Operand src);
x64_Instruction x64_sub(x64_Operand dst, x64_Operand src);
x64_Instruction x64_imul(x64_Operand src);
x64_Instruction x64_idiv(x64_Operand src);

void x64_instruction_emit(x64_Instruction I,
                          String *restrict buffer,
                          Context *restrict context);

#endif // !EXP_BACKEND_X64_INSTRUCTION_H
