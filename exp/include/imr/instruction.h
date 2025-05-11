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

/*
 * #NOTE Control flow instructions which modify the instruction pointer
 * are going to be implemented in terms of basic blocks. That is, when
 * we have a jump instruction, the tag we jump to will not be an instruction
 * within the current basic block, but the index of the target basic block
 * within the set of basic blocks composing the body of a function.
 * Essentially, the instruction pointer will not be a single integer, it will
 * be two integers, which basic block is being evaluated, and which instruction
 * within that basic block.
 * Evaluation will always start as basic block 0, and each basic block must end
 * with a jump instruction of some variety. (return counts as a jump
 * instruction)
 */

/*
 * #NOTE The current design only allows the definition of names to occur by
 * way of the "let" or "fn" instructions. This forces each other instruction
 * with a destination (add, sub, mul, etc...) to use a local temporary to
 * communicate the result to the "let" instruction. It would be faster and
 * result in smaller bytecode if we allowed any instruction with a destination
 * to act as a "let" expression, by way of a label being the target destination.
 * example source code:
 ```
    let x = 5 + 7;
 ```
 * is currently compiled to the following bytecode:
 ```
    add %0, 5, 7;
    let %x, %0;
 ```
 * this would instead be translated to:
 ```
    add %x, 5, 7;
 ```
 * this would have no effect for statements which define a name to equal to a
 * constant directly.
 * such as:
 * `let x = 5;`
 * `let x = (1, 2);`
 * These would need to be compiled into let instructions.
 * So the change would be mostly within the parser. plus modifying
 * the evaluator to handle non SSA operand As.
 */

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

inline Operand operand_A(Instruction instruction) {
    return operand(instruction.A_kind, instruction.A_data);
}

inline Operand operand_B(Instruction instruction) {
    return operand(instruction.B_kind, instruction.B_data);
}

inline Operand operand_C(Instruction instruction) {
    return operand(instruction.C_kind, instruction.C_data);
}

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
