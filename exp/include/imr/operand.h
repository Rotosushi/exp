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
#ifndef EXP_IMR_OPERAND_H
#define EXP_IMR_OPERAND_H

#include "adt/string.h"

typedef enum OperandKind : u8 {
    OPERAND_KIND_SSA,
    OPERAND_KIND_LABEL,
    // #TODO: OPERAND_KIND_SCALAR,
    OPERAND_KIND_I32,
    OPERAND_KIND_CONSTANT,
} OperandKind;

typedef union OperandPayload {
    u32 ssa;
    i32 i32_;
    u32 constant;
    u32 label;
} OperandData;

typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

Operand operand_construct(OperandKind kind, OperandData data);
Operand operand_ssa(u32 ssa);
Operand operand_constant(u32 constant);
Operand operand_i32(i32 i32_);
Operand operand_label(u32 label);
bool operand_equality(Operand A, Operand B);

struct Context;
void print_operand(String *buffer,
                   OperandKind kind,
                   OperandData data,
                   struct Context *context);

#endif // EXP_IMR_OPERAND_H
