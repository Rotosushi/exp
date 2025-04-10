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
#ifndef EXP_BACKEND_X64_BYTECODE_H
#define EXP_BACKEND_X64_BYTECODE_H

#include "codegen/x64/imr/instruction.h"
#include "support/string.h"

typedef struct x64_Bytecode {
    u64              length;
    u64              capacity;
    x64_Instruction *buffer;
} x64_Bytecode;

x64_Bytecode x64_bytecode_create();
void         x64_bytecode_destroy(x64_Bytecode *restrict bc);

u64 x64_bytecode_current_offset(x64_Bytecode *restrict bc);

void x64_bytecode_insert(x64_Bytecode *restrict bc,
                         x64_Instruction I,
                         u64             offset);
void x64_bytecode_prepend(x64_Bytecode *restrict bc, x64_Instruction I);
void x64_bytecode_append(x64_Bytecode *restrict bc, x64_Instruction I);

void x64_bytecode_emit(x64_Bytecode *restrict bc,
                       String *restrict buffer,
                       Context *restrict context);
#endif // !EXP_BACKEND_X64_BYTECODE_H
