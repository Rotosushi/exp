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
#ifndef EXP_BACKEND_X64_OPERAND_H
#define EXP_BACKEND_X64_OPERAND_H

#include "codegen/x86/imr/allocation.h"

typedef enum x64_OperandKind : u8 {
    X64_OPERAND_KIND_GPR,
    X64_OPERAND_KIND_ADDRESS,
    X64_OPERAND_KIND_LABEL,
    X64_OPERAND_KIND_CONSTANT,
    X64_OPERAND_KIND_IMMEDIATE,
} x64_OperandKind;

typedef union x64_OperandData {
    x86_64_GPR  gpr;
    x86_Address address;
    u32         label;
    u32         constant;
    i64         immediate;
} x64_OperandData;

typedef struct x64_Operand {
    x64_OperandKind kind;
    union {
        x86_64_GPR  gpr;
        x86_Address address;
        u32         label;
        u32         constant;
        i64         immediate;
    };
} x64_Operand;

x64_Operand x64_operand_gpr(x86_64_GPR gpr);
x64_Operand x64_operand_address(x86_Address address);
x64_Operand x64_operand_location(x64_Location location);
x64_Operand x64_operand_alloc(x86_Allocation *alloc);
x64_Operand x64_operand_constant(u32 index);
x64_Operand x64_operand_label(u32 index);
x64_Operand x64_operand_immediate(i64 value);

#endif // !EXP_BACKEND_X64_OPERAND_H
