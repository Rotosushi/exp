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
#ifndef EXP_TARGETS_X86_64_INSTRUCTIONS_OPERAND_H
#define EXP_TARGETS_X86_64_INSTRUCTIONS_OPERAND_H

#include "adt/string.h"
#include "targets/x86_64/address.h"
#include "utility/constant_string.h"

typedef enum x86_64_OperandKind : u8 {
    X86_64_OPERAND_KIND_GPR,
    X86_64_OPERAND_KIND_ADDRESS,
    X86_64_OPERAND_KIND_I64,
    X86_64_OPERAND_KIND_LABEL,
} x86_64_OperandKind;

typedef union x86_64_OperandData {
    x86_64_GPR gpr;
    x86_64_Address address;
    i64 i64_;
    ConstantString *label;
} x86_64_OperandData;

typedef struct x86_64_Operand {
    x86_64_OperandKind kind;
    x86_64_OperandData data;
} x86_64_Operand;

x86_64_Operand x86_64_operand_gpr(x86_64_GPR gpr);
x86_64_Operand x86_64_operand_address(x86_64_Address address);
x86_64_Operand x86_64_operand_i64(i64 i64_);
x86_64_Operand x86_64_operand_label(ConstantString *label);
u8 x86_64_operand_size(x86_64_Operand operand);
void print_x86_64_operand(String *buffer, x86_64_Operand operand);

#endif // EXP_TARGETS_X86_64_INSTRUCTIONS_OPERAND_H
