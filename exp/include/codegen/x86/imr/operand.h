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
#ifndef EXP_BACKEND_X86_OPERAND_H
#define EXP_BACKEND_X86_OPERAND_H

#include "codegen/x86/imr/location.h"
#include "support/constant_string.h"

typedef enum x86_OperandKind : u8 {
    X86_OPERAND_KIND_LOCATION,
    X86_OPERAND_KIND_LABEL,
    X86_OPERAND_KIND_NIL,
    X86_OPERAND_KIND_BOOL,
    X86_OPERAND_KIND_U8,
    X86_OPERAND_KIND_U16,
    X86_OPERAND_KIND_U32,
    X86_OPERAND_KIND_U64,
    X86_OPERAND_KIND_I8,
    X86_OPERAND_KIND_I16,
    X86_OPERAND_KIND_I32,
    X86_OPERAND_KIND_I64,
} x86_OperandKind;

typedef union x86_OperandData {
    x86_Location          location;
    ConstantString const *label;
    bool                  nil;
    bool                  bool_;
    u8                    u8_;
    u16                   u16_;
    u32                   u32_;
    u64                   u64_;
    i8                    i8_;
    i16                   i16_;
    i32                   i32_;
    i64                   i64_;
} x86_OperandData;

typedef struct x86_Operand {
    x86_OperandKind kind;
    x86_OperandData data;
} x86_Operand;

x86_Operand x86_operand(x86_OperandKind kind, x86_OperandData data);
x86_Operand x86_operand_location_gpr(x86_GPR gpr);
x86_Operand x86_operand_location_address(x86_GPR base, i32 offset);
x86_Operand x86_operand_location_address_indexed(x86_GPR base,
                                                 x86_GPR index,
                                                 u8      scale,
                                                 i32     offset);
x86_Operand x86_operand_label(ConstantString const *cs);
x86_Operand x86_operand_nil();
x86_Operand x86_operand_bool(bool bool_);
x86_Operand x86_operand_u8(u8 u8_);
x86_Operand x86_operand_u16(u16 u16_);
x86_Operand x86_operand_u32(u32 u32_);
x86_Operand x86_operand_u64(u64 u64_);
x86_Operand x86_operand_i8(i8 i8_);
x86_Operand x86_operand_i16(i16 i16_);
x86_Operand x86_operand_i32(i32 i32_);
x86_Operand x86_operand_i64(i64 value);

void print_x86_operand(String *restrict buffer, x86_Operand operand);

#endif // !EXP_BACKEND_X86_OPERAND_H
