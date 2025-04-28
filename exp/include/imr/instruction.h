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
#ifndef EXP_IMR_INSTRUCTION_H
#define EXP_IMR_INSTRUCTION_H
#include "imr/operand.h"

/**
 * @brief the valid opcodes for instructions
 */
typedef enum Opcode : u8 {
    OPCODE_RET,
    OPCODE_CALL,
    OPCODE_LET,
    OPCODE_NEG,
    OPCODE_DOT,
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_MUL,
    OPCODE_DIV,
    OPCODE_MOD,
} Opcode;

typedef struct Instruction {
    Opcode      opcode;
    OperandKind A_kind;
    OperandKind B_kind;
    OperandKind C_kind;
    OperandData A_data;
    OperandData B_data;
    OperandData C_data;
} Instruction;

Instruction instruction_return(Operand result);
Instruction instruction_call(Operand dst, Operand label, Operand args);
Instruction instruction_let(Operand dst, Operand src);
Instruction instruction_neg(Operand dst, Operand src);
Instruction instruction_dot(Operand dst, Operand src, Operand index);
Instruction instruction_add(Operand dst, Operand left, Operand right);
Instruction instruction_sub(Operand dst, Operand left, Operand right);
Instruction instruction_mul(Operand dst, Operand left, Operand right);
Instruction instruction_div(Operand dst, Operand left, Operand right);
Instruction instruction_mod(Operand dst, Operand left, Operand right);

struct Context;
void print_instruction(String *restrict string,
                       Instruction instruction,
                       struct Context *restrict context);

#endif // !EXP_IMR_INSTRUCTION_H
