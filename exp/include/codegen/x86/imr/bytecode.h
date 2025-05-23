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
#ifndef EXP_BACKEND_X86_BYTECODE_H
#define EXP_BACKEND_X86_BYTECODE_H

#include "codegen/x86/imr/instruction.h"
#include "support/string.h"

typedef struct x86_Bytecode {
    u64              length;
    u64              capacity;
    x86_Instruction *buffer;
} x86_Bytecode;

x86_Bytecode x86_bytecode_create();
void         x86_bytecode_destroy(x86_Bytecode *restrict bc);

u64 x86_bytecode_current_offset(x86_Bytecode *restrict bc);

void x86_bytecode_insert(x86_Bytecode *restrict bc,
                         x86_Instruction I,
                         u64             offset);
void x86_bytecode_prepend(x86_Bytecode *restrict bc, x86_Instruction I);
void x86_bytecode_append(x86_Bytecode *restrict bc, x86_Instruction I);

void x86_bytecode_emit(x86_Bytecode *restrict bc,
                       String *restrict buffer,
                       Context *restrict context);

#endif // !EXP_BACKEND_X86_BYTECODE_H

