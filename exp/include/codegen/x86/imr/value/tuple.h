// Copyright (C) 2025 Cade Weinberg
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
#ifndef EXP_CODEGEN_X86_IMR_BODY_TUPLE_H
#define EXP_CODEGEN_X86_IMR_BODY_TUPLE_H

#include "codegen/x86/imr/operand.h"

typedef struct x86_Tuple {
    u32          length;
    u32          capacity;
    x86_Operand *buffer;
} x86_Tuple;

void x86_tuple_create(x86_Tuple *restrict tuple);
void x86_tuple_destroy(x86_Tuple *restrict tuple);

void x86_tuple_reserve(x86_Tuple *restrict tuple, u32 size);

void x86_tuple_append(x86_Tuple *restrict tuple, x86_Operand operand);

x86_Operand *x86_tuple_at(x86_Tuple *restrict tuple, u32 index);

#endif // !EXP_CODEGEN_X86_IMR_BODY_TUPLE_H
