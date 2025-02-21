// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file targets/x86_64/imr/instruction.h
 */

#ifndef EXP_TARGETS_X86_64_IMR_INSTRUCTION_H
#define EXP_TARGETS_X86_64_IMR_INSTRUCTION_H

#include "targets/x86_64/imr/operand.h"
#include "utility/string.h"

/*
  We want to model the x86_64 instructions we generate
  in memory, so we can compare different instruction
  sequences and choose the most efficient one.

  to this end we have more attributes that we need to account for.

  --

  generally speaking x86_64 instructions use the opcode
  and the first operand together to determine the
  size of the operands, in terms of what we emit for
  gnu-as we will be explicit about the size of the
  operands with the instruction size mnemonic.
*/

typedef enum x86_64_Opcode : u16 {
    X86_64_OPCODE_RET,
    X86_64_OPCODE_CALL,
    X86_64_OPCODE_PUSH,
    X86_64_OPCODE_POP,
    X86_64_OPCODE_MOV,
    X86_64_OPCODE_LEA,
    X86_64_OPCODE_NEG,
    X86_64_OPCODE_ADD,
    X86_64_OPCODE_SUB,
    X86_64_OPCODE_IMUL,
    X86_64_OPCODE_IDIV,
} x86_64_Opcode;

/*
 * #TODO: x86_64 instructions can have more than two operands
 */

typedef struct x86_64_Instruction {
    x86_64_Opcode opcode;
    x86_64_OperandKind A_kind;
    x86_64_OperandKind B_kind;
    x86_64_OperandData A_data;
    x86_64_OperandData B_data;
} x86_64_Instruction;

x86_64_Instruction x86_64_ret();
x86_64_Instruction x86_64_call(x86_64_Operand label);
x86_64_Instruction x86_64_push(x86_64_Operand source);
x86_64_Instruction x86_64_pop(x86_64_Operand target);
x86_64_Instruction x86_64_mov(x86_64_Operand target, x86_64_Operand source);
x86_64_Instruction x86_64_lea(x86_64_Operand target, x86_64_Operand source);
x86_64_Instruction x86_64_neg(x86_64_Operand target);
x86_64_Instruction x86_64_add(x86_64_Operand target, x86_64_Operand source);
x86_64_Instruction x86_64_sub(x86_64_Operand target, x86_64_Operand source);
x86_64_Instruction x86_64_imul(x86_64_Operand source);
x86_64_Instruction x86_64_idiv(x86_64_Operand source);

struct Context;
void print_x86_64_instruction(String *buffer, x86_64_Instruction instruction,
                              struct Context *context);

#endif // EXP_TARGETS_X86_64_IMR_INSTRUCTION_H
