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

#include "adt/string.h"
#include "backend/x64/operand.h"
#include "env/context.h"

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

  generally speaking x64 instructions have the opcode
  and then the first operand together determine the
  size of the operands.

  so we are simplifying this for now, just to represent
  what we need to. all operands are going to be 64 bit.

  #TODO emiting instructions needs to take into account the size of
  their operands, such that we can append the correct mnemonic
  suffix to the x64 assembly instruction.

*/

typedef enum x64_Opcode {
  X64OPC_RET,
  X64OPC_PUSH,
  X64OPC_POP,
  X64OPC_MOV,
  X64OPC_NEG,
  X64OPC_ADD,
  X64OPC_SUB,
  X64OPC_IMUL,
  X64OPC_IDIV,
} x64_Opcode;

typedef struct x64_Instruction {
  unsigned opcode : 8;
  unsigned Afmt   : 3;
  unsigned Bfmt   : 3;
  unsigned        : 2;
  unsigned A      : 16;
  unsigned B      : 16;
} x64_Instruction;

x64_Instruction x64_inst(x64_Opcode opcode);
x64_Instruction x64_inst_A(x64_Opcode opcode, x64_Operand A);
x64_Instruction x64_inst_AB(x64_Opcode opcode, x64_Operand A, x64_Operand B);

void x64_inst_emit(x64_Instruction I,
                   String *restrict buffer,
                   Context *restrict context);

#endif // !EXP_BACKEND_X64_INSTRUCTION_H