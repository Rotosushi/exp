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

#include "utility/int_types.h"
#include "utility/io.h"

typedef enum OperandFormat {
  OPRFMT_SSA,
  OPRFMT_CONSTANT,
  OPRFMT_IMMEDIATE,
  OPRFMT_LABEL,
} OperandFormat;

typedef struct Operand {
  unsigned format : 3;
  unsigned common : 16;
} Operand;

Operand operand_create(OperandFormat format, u16 common);
Operand operand_ssa(u16 ssa);
Operand operand_constant(u16 index);
Operand operand_immediate(u16 imm);
Operand operand_label(u16 idx);
bool operand_equality(Operand A, Operand B);
void print_operand(Operand operand, FILE *restrict file);

#endif // EXP_IMR_OPERAND_H