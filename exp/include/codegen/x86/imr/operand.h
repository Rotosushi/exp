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

#include "codegen/x86/imr/allocation.h"
#include "imr/value.h"
#include "support/constant_string.h"

typedef enum x86_OperandKind : u8 {
    X86_OPERAND_KIND_GPR,
    X86_OPERAND_KIND_ADDRESS,
    X86_OPERAND_KIND_LABEL,
    X86_OPERAND_KIND_CONSTANT,
    X86_OPERAND_KIND_IMMEDIATE,
} x86_OperandKind;

typedef union x86_OperandData {
    x86_GPR               gpr;
    x86_Address           address;
    ConstantString const *label;
    Value const          *constant;
    i64                   i64_;
} x86_OperandData;

typedef struct x86_Operand {
    x86_OperandKind kind;
    x86_OperandData data;
} x86_Operand;

x86_Operand x86_operand_gpr(x86_GPR gpr);
x86_Operand x86_operand_address(x86_Address address);
x86_Operand x86_operand_location(x86_Location location);
x86_Operand x86_operand_alloc(x86_Allocation *alloc);
x86_Operand x86_operand_constant(Value const *constant);
x86_Operand x86_operand_label(ConstantString const *cs);
x86_Operand x86_operand_i64(i64 value);

#endif // !EXP_BACKEND_X86_OPERAND_H
