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

typedef enum x64_OperandFormat : u8 {
  X64OPRFMT_GPR,
  X64OPRFMT_ADDRESS,
  X64OPRFMT_IMMEDIATE,
  X64OPRFMT_LABEL,
} x64_OperandFormat;

typedef struct x64_Operand {
  x64_OperandFormat format;
  union {
    x64_GPR gpr;
    x64_Address address;
    u64 index;
    i64 immediate;
  };
} x64_Operand;

x64_Operand x64_operand_gpr(x64_GPR gpr);
x64_Operand x64_operand_address(x64_Address address);
x64_Operand x64_operand_location(x64_Location location);
x64_Operand x64_operand_alloc(x64_Allocation *alloc);
x64_Operand x64_operand_immediate(i64 value);
x64_Operand x64_operand_label(u64 index);

#endif // !EXP_BACKEND_X64_OPERAND_H
