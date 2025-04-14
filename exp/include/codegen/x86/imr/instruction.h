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
    x64_Opcode  opcode;
    x86_Operand A;
    x86_Operand B;
} x64_Instruction;

x64_Instruction x64_ret();
x64_Instruction x64_call(x86_Operand label);
x64_Instruction x64_push(x86_Operand src);
x64_Instruction x64_pop(x86_Operand dst);
x64_Instruction x64_mov(x86_Operand dst, x86_Operand src);
x64_Instruction x64_lea(x86_Operand dst, x86_Operand src);

x64_Instruction x64_neg(x86_Operand dst);
x64_Instruction x64_add(x86_Operand dst, x86_Operand src);
x64_Instruction x64_sub(x86_Operand dst, x86_Operand src);
x64_Instruction x64_imul(x86_Operand src);
x64_Instruction x64_idiv(x86_Operand src);

void x64_instruction_emit(x64_Instruction I,
                          String *restrict buffer,
                          Context *restrict context);

#endif // !EXP_BACKEND_X64_INSTRUCTION_H
