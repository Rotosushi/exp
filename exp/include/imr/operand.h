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
#include <stdbool.h>
#include <stdio.h>

#include "support/scalar.h"

typedef enum OperandKind : u8 {
    OPERAND_KIND_SSA       = 0x0,
    OPERAND_KIND_CONSTANT  = 0x1,
    OPERAND_KIND_LABEL     = 0x2,
    OPERAND_KIND_IMMEDIATE = 0x3,
} OperandKind;

typedef union OperandPayload {
    u32 ssa;
    u32 constant;
    u32 label;
    i64 immediate;
} OperandData;

typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

struct Context;

Operand operand(OperandKind kind, OperandData data);
Operand operand_ssa(u32 ssa);
Operand operand_constant(u32 index);
Operand operand_label(u32 index);
Operand operand_immediate(i64 immediate);
bool operand_equality(Operand A, Operand B);
void print_operand(Operand operand,
                   FILE *restrict file,
                   struct Context *restrict context);

#endif // EXP_IMR_OPERAND_H
