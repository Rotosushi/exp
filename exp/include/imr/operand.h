// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file imr/operand.h
 */

#ifndef EXP_IMR_OPERAND_H
#define EXP_IMR_OPERAND_H

#include "imr/scalar.h"
#include "utility/string.h"

typedef enum OperandKind : u8 {
    OPERAND_UNINITIALIZED,
    OPERAND_REGISTER,
    OPERAND_STACK,
    OPERAND_SCALAR_NIL,
    OPERAND_SCALAR_BOOL,
    OPERAND_SCALAR_U8,
    OPERAND_SCALAR_U16,
    OPERAND_SCALAR_U32,
    OPERAND_SCALAR_U64,
    OPERAND_SCALAR_I8,
    OPERAND_SCALAR_I16,
    OPERAND_SCALAR_I32,
    OPERAND_SCALAR_I64,
} OperandKind;

typedef union OperandData {
    u8 register_;
    u16 stack;
    bool nil;
    bool bool_;
    u8 u8_;
    u16 u16_;
    u32 u32_;
    u64 u64_;
    i8 i8_;
    i16 i16_;
    i32 i32_;
    i64 i64_;
} OperandData;

/**
 * @brief Represents an operand to an Instruction
 */
typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

Operand operand_construct(OperandKind kind, OperandData data);
Operand operand_uninitialized();
Operand operand_register(u8 register_);
Operand operand_stack(u16 stack);
Operand operand_scalar(Scalar scalar);

Scalar operand_as_scalar(Operand operand);

bool operand_equality(Operand A, Operand B);

void print_operand(String *buffer, Operand operand);

#endif // EXP_IMR_OPERAND_H
