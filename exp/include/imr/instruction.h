// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_INSTRUCTION_H
#define EXP_IMR_INSTRUCTION_H
#include "imr/operand.h"

/**
 * @brief the valid opcodes for instructions
 */
typedef enum Opcode : u8 {
    // Memory
    OPCODE_LOAD,
    // @todo
    // OPCODE_STORE,

    // control flow
    OPCODE_RET,
    OPCODE_CALL,
    // @todo:
    // OPCODE_JUMP,
    // OPCODE_JUMP_IF_EQUAL,

    // Unops
    OPCODE_NEG,

    // Binops
    OPCODE_DOT,
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_MUL,
    OPCODE_DIV,
    OPCODE_MOD,
} Opcode;

/**
 * @brief represents a single instruction in the IR
 */
typedef struct Instruction {
    Opcode opcode;
    OperandKind A_kind;
    OperandKind B_kind;
    OperandKind C_kind;
    OperandData A_data;
    OperandData B_data;
    OperandData C_data;
} Instruction;

Instruction instruction_return(Operand dst, Operand result);
Instruction instruction_call(Operand dst, Operand label, Operand args);
Instruction instruction_dot(Operand dst, Operand src, Operand index);
Instruction instruction_load(Operand dst, Operand src);
Instruction instruction_negate(Operand dst, Operand src);
Instruction instruction_add(Operand dst, Operand left, Operand right);
Instruction instruction_subtract(Operand dst, Operand left, Operand right);
Instruction instruction_multiply(Operand dst, Operand left, Operand right);
Instruction instruction_divide(Operand dst, Operand left, Operand right);
Instruction instruction_modulus(Operand dst, Operand left, Operand right);

#endif // !EXP_IMR_INSTRUCTION_H
