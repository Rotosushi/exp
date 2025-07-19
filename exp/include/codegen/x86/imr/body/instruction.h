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

#include "codegen/x86/imr/body/operand.h"

typedef enum x86_Opcode : u8 {
    X86_OPCODE_RET,
    X86_OPCODE_CALL,
    X86_OPCODE_JMP,
    X86_OPCODE_PUSH,
    X86_OPCODE_POP,
    X86_OPCODE_MOV,
    X86_OPCODE_LEA,
    X86_OPCODE_AND,
    X86_OPCODE_OR,
    X86_OPCODE_XOR,
    X86_OPCODE_NOT,
    X86_OPCODE_NEG,
    X86_OPCODE_ADD,
    X86_OPCODE_SUB,
    X86_OPCODE_IMUL,
    X86_OPCODE_IDIV,
} x86_Opcode;

typedef struct x86_Instruction {
    x86_Opcode      opcode;
    x86_OperandKind A_kind;
    x86_OperandKind B_kind;
    // 5 bytes padding
    x86_OperandData A_data;
    x86_OperandData B_data;
} x86_Instruction;

x86_Instruction x86_ret();
x86_Instruction x86_call(x86_Operand label);

x86_Instruction x86_jmp(x86_Operand dst);

x86_Instruction x86_push(x86_Operand src);
x86_Instruction x86_pop(x86_Operand dst);

x86_Instruction x86_mov(x86_Operand dst, x86_Operand src);
x86_Instruction x86_lea(x86_Operand dst, x86_Operand src);

x86_Instruction x86_and(x86_Operand dst, x86_Operand src);
x86_Instruction x86_or(x86_Operand dst, x86_Operand src);
x86_Instruction x86_xor(x86_Operand dst, x86_Operand src);
x86_Instruction x86_not(x86_Operand dst);

x86_Instruction x86_neg(x86_Operand dst);
x86_Instruction x86_add(x86_Operand dst, x86_Operand src);
x86_Instruction x86_sub(x86_Operand dst, x86_Operand src);
x86_Instruction x86_imul(x86_Operand src);
x86_Instruction x86_idiv(x86_Operand src);

void print_x86_instruction(String *restrict buffer,
                           x86_Instruction instruction);

#endif // !EXP_BACKEND_X64_INSTRUCTION_H
