// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_OPERAND_H
#define EXP_IMR_OPERAND_H

#include "utility/string.h"

typedef enum OperandKind : u8 {
    OPERAND_REGISTER,
    OPERAND_STACK,
} OperandKind;

typedef union OperandPayload {
    u8 register_;
    u32 stack;
} OperandData;

typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

Operand operand_construct(OperandKind kind, OperandData data);
Operand operand_register(u8 register_);
Operand operand_stack(u32 stack);

bool operand_equality(Operand A, Operand B);

struct Context;
void print_operand(String *buffer, Operand operand, struct Context *context);

#endif // EXP_IMR_OPERAND_H
