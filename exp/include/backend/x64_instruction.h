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
#include "backend/x64_allocation.h"
#include "backend/x64_gpr.h"
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

*/

typedef enum X64Opcode {
  X64OPC_RET,
  X64OPC_PUSH,
  X64OPC_POP,
  X64OPC_MOV,
  X64OPC_NEG,
  X64OPC_ADD,
  X64OPC_SUB,
  X64OPC_IMUL,
  X64OPC_IDIV,
} X64Opcode;

/*
  #TODO:
  we need to specify memory operands more generally
  to support pointers. however, a memory operand requires
  more than one datum, the register holding the address
  and the offset from that address. and more. this is
  a complexity we are deliberately avoiding right now.
*/

typedef enum X64OperandFormat {
  X64OPRFMT_GPR,
  X64OPRFMT_STACK,
  X64OPRFMT_CONSTANT,
  X64OPRFMT_IMMEDIATE,
} X64OperandFormat;

typedef struct X64Operand {
  unsigned format : 3;
  unsigned common : 16;
} X64Operand;

X64Operand x64opr_gpr(u16 gpr);
X64Operand x64opr_stack(u16 offset);
X64Operand x64opr_alloc(X64Allocation *alloc);
X64Operand x64opr_constant(u16 idx);
X64Operand x64opr_immediate(u16 n);

typedef struct X64Instruction {
  unsigned opcode : 8;
  unsigned Afmt   : 3;
  unsigned Bfmt   : 3;
  unsigned        : 2;
  unsigned A      : 16;
  unsigned B      : 16;
} X64Instruction;

X64Instruction x64inst(X64Opcode opcode);
X64Instruction x64inst_A(X64Opcode opcode, X64Operand A);
X64Instruction x64inst_AB(X64Opcode opcode, X64Operand A, X64Operand B);

void x64inst_emit(X64Instruction I,
                  String *restrict buffer,
                  Context *restrict context);

#endif // !EXP_BACKEND_X64_INSTRUCTION_H