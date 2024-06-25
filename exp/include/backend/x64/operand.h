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
  #TODO:
  we need to specify memory operands more generally
  to support pointers. however, a memory operand requires
  more than one datum, the register holding the address
  and the offset from that address. and more. this is
  a complexity we are deliberately avoiding right now.
*/

typedef enum x64_OperandFormat {
  X64OPRFMT_GPR,
  X64OPRFMT_STACK,
  X64OPRFMT_CONSTANT,
  X64OPRFMT_IMMEDIATE,
  X64OPRFMT_LABEL,
} x64_OperandFormat;

typedef struct x64_Operand {
  unsigned format : 3;
  unsigned common : 16;
} x64_Operand;

x64_Operand x64_operand_gpr(u16 gpr);
x64_Operand x64_operand_stack(u16 offset);
x64_Operand x64_operand_location(x64_Location location);
x64_Operand x64_operand_alloc(x64_Allocation *alloc);
x64_Operand x64_operand_constant(u16 idx);
x64_Operand x64_operand_immediate(u16 value);
x64_Operand x64_operand_label(u16 idx);

#endif // !EXP_BACKEND_X64_OPERAND_H