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

typedef enum OperandFormat : u8 {
  OPRFMT_SSA,
  OPRFMT_VALUE,
  OPRFMT_IMMEDIATE,
  OPRFMT_LABEL,
} OperandFormat;

typedef struct Operand {
  OperandFormat format;
  union {
    u64 ssa;
    u64 index;
    i64 immediate;
  };
} Operand;

Operand operand_ssa(u64 ssa);
Operand operand_constant(u64 index);
Operand operand_immediate(i64 imm);
Operand operand_label(u64 idx);
bool operand_equality(Operand A, Operand B);
void print_ssa(u64 ssa, FILE *restrict file);
void print_operand(Operand operand, FILE *restrict file);

#endif // EXP_IMR_OPERAND_H