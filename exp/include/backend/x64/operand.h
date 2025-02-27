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

#include "backend/x64/allocation.h"

/*
 * #TODO:
 *   typedef enum x64_OperandKind : u8 {
 *     X64OPRFMT_LOCATION,
 *     X64OPRFMT_CONSTANT,
 *     X64OPRFMT_IMMEDIATE,
 *     X64OPRFMT_LABEL,
 *   } x64_OperandKind;
 *   typedef struct x64_Operand {
 *     unsigned kind : 8;
 *     unsigned      : 8;
 *     unsigned data : 16;
 *   } x64_Operand
 */

typedef enum x64_OperandFormat : u8 {
    X64_OPERAND_KIND_GPR,
    X64_OPERAND_KIND_ADDRESS,
    X64_OPERAND_KIND_IMMEDIATE,
    X64_OPERAND_KIND_LABEL,
    X64_OPERAND_KIND_CONSTANT,
} x64_OperandFormat;

typedef struct x64_Operand {
    x64_OperandFormat format;
    union {
        x64_GPR gpr;
        x64_Address address;
        i16 immediate;
        u16 label;
        u16 constant;
    };
} x64_Operand;

x64_Operand x64_operand_gpr(x64_GPR gpr);
x64_Operand x64_operand_address(x64_Address address);
x64_Operand x64_operand_location(x64_Location location);
x64_Operand x64_operand_alloc(x64_Allocation *alloc);
x64_Operand x64_operand_immediate(i16 value);
x64_Operand x64_operand_constant(u16 index);
x64_Operand x64_operand_label(u16 index);

#endif // !EXP_BACKEND_X64_OPERAND_H
