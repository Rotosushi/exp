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
#include "utility/constant_string.h"

// #TODO: Operands are 64 bits, so we should allow any scalar type <= 64 bits in
//  size to be immediately available within an operand. This should speed up
//  working with values of any of these types within the compiler, or any
//  interpretation we do with the bytecode. And since we expanded OperandKind
//  to 2 bytes, we have 2^16 = 65,536 possible kinds of operand available.

typedef enum OperandKind : u16 {
    OPERAND_KIND_SSA,
    // #TODO:
    //  OPERAND_KIND_I8,
    //  OPERAND_KIND_I16,
    //  OPERAND_KIND_I32
    OPERAND_KIND_I64,
    //  OPERAND_KIND_U8,
    //  OPERAND_KIND_U16,
    //  OPERAND_KIND_U32,
    //  OPERAND_KIND_U64,
    //  OPERAND_KIND_F32,
    //  OPERAND_KIND_F64,
    OPERAND_KIND_CONSTANT,
    OPERAND_KIND_LABEL,
} OperandKind;

struct Value;

typedef union OperandPayload {
    u64 ssa;
    i64 i64_;
    struct Value *constant;
    ConstantString *label;
} OperandData;

typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

Operand operand_construct(OperandKind kind, OperandData data);
Operand operand_ssa(u64 ssa);
Operand operand_constant(struct Value *constant);
Operand operand_i64(i64 i64_);
Operand operand_label(ConstantString *label);
bool operand_equality(Operand A, Operand B);

struct Context;
void print_operand(String *buffer,
                   OperandKind kind,
                   OperandData data,
                   struct Context *context);

#endif // EXP_IMR_OPERAND_H
