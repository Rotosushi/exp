// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_OPERAND_H
#define EXP_IMR_OPERAND_H

#include "imr/scalar.h"
#include "utility/string.h"
#include <stdint.h>

typedef enum OperandKind : u8 {
    OPERAND_SSA,
    OPERAND_LABEL,
    OPERAND_SCALAR,
    OPERAND_CONSTANT,
} OperandKind;

typedef union OperandPayload {
    u32 ssa;
    u32 constant;
    u32 label;
    Scalar scalar;
} OperandData;

typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

Operand operand_construct(OperandKind kind, OperandData data);
Operand operand_ssa(u32 ssa);
Operand operand_constant(u32 constant);
Operand operand_scalar(Scalar scalar);
Operand operand_label(u32 label);
bool operand_equality(Operand A, Operand B);

struct Context;
void print_operand(String *buffer, Operand operand, struct Context *context);

#endif // EXP_IMR_OPERAND_H
