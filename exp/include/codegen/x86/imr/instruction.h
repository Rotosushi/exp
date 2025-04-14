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

#include "codegen/x86/imr/operand.h"
#include "env/context.h"
#include "support/string.h"

typedef enum x86_Opcode : u8 {
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
} x86_Opcode;

typedef struct x86_Instruction {
    x86_Opcode  opcode;
    x86_Operand A;
    x86_Operand B;
} x86_Instruction;

x86_Instruction x86_ret();
x86_Instruction x86_call(x86_Operand label);
x86_Instruction x86_push(x86_Operand src);
x86_Instruction x86_pop(x86_Operand dst);
x86_Instruction x86_mov(x86_Operand dst, x86_Operand src);
x86_Instruction x86_lea(x86_Operand dst, x86_Operand src);

x86_Instruction x86_neg(x86_Operand dst);
x86_Instruction x86_add(x86_Operand dst, x86_Operand src);
x86_Instruction x86_sub(x86_Operand dst, x86_Operand src);
x86_Instruction x86_imul(x86_Operand src);
x86_Instruction x86_idiv(x86_Operand src);

void x86_instruction_emit(x86_Instruction I,
                          String *restrict buffer,
                          Context *restrict context);

#endif // !EXP_BACKEND_X64_INSTRUCTION_H
