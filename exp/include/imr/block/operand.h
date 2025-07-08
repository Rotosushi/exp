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

#include "support/constant_string.h"
#include "support/scalar.h"
#include "support/string.h"

typedef enum OperandKind : u8 {
    OPERAND_KIND_SSA,
    OPERAND_KIND_CONSTANT,
    OPERAND_KIND_LABEL,
    OPERAND_KIND_NIL,
    OPERAND_KIND_BOOL,
    OPERAND_KIND_U8,
    OPERAND_KIND_U16,
    OPERAND_KIND_U32,
    OPERAND_KIND_U64,
    OPERAND_KIND_I8,
    OPERAND_KIND_I16,
    OPERAND_KIND_I32,
    OPERAND_KIND_I64,
} OperandKind;

struct Value;

typedef union OperandData {
    u32                   ssa;
    struct Value const   *constant;
    ConstantString const *label;
    u8                    nil;
    bool                  bool_;
    u8                    u8_;
    u16                   u16_;
    u32                   u32_;
    u64                   u64_;
    i8                    i8_;
    i16                   i16_;
    i32                   i32_;
    i64                   i64_;
} OperandData;

typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

struct Context;

Operand operand(OperandKind kind, OperandData data);
Operand operand_ssa(u32 local);
Operand operand_constant(struct Value const *constant);
Operand operand_label(ConstantString const *label);
Operand operand_nil();
Operand operand_bool(bool bool_);
Operand operand_u8(u8 u8_);
Operand operand_u16(u16 u16_);
Operand operand_u32(u32 u32_);
Operand operand_u64(u64 u64_);
Operand operand_i8(i8 i8_);
Operand operand_i16(i16 i16_);
Operand operand_i32(i32 i32_);
Operand operand_i64(i64 i64_);

bool operand_equality(Operand A, Operand B);
bool operand_is_index(Operand A);
u64  operand_as_index(Operand A);

void print_operand(String *restrict string,
                   Operand operand,
                   struct Context *restrict context);

#endif // EXP_IMR_OPERAND_H
