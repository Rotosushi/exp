// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_OPERAND_H
#define EXP_IMR_OPERAND_H

#include "utility/string.h"

typedef enum OperandKind : u8 {
    OPERAND_UNINITIALIZED = 0x0,
    OPERAND_REGISTER      = 0x1,
    OPERAND_STACK         = 0x2,
} OperandKind;

/**
 * @note I am debating changing stack to a u16,
 * as this allows an Instruction to be 8 bytes.
 * This limits the number of stack slots to 65536
 * which feels like it's enough for a single function.
 * The total stack for the whole program can still be
 * 4GB.
 */
typedef union OperandPayload {
    u8 register_;
    u16 stack;
} OperandData;

/**
 * @brief Represents an operand to an Instruction
 *
 * @note Operands can be either a register or a stack slot
 */
typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

Operand operand_construct(OperandKind kind, OperandData data);
Operand operand_uninitialized();
Operand operand_register(u8 register_);
Operand operand_stack(u16 stack);

bool operand_equality(Operand A, Operand B);

void print_operand(String *buffer, Operand operand);

#endif // EXP_IMR_OPERAND_H
