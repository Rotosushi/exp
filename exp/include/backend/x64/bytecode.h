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

#include "adt/string.h"
#include "backend/x64/instruction.h"

typedef struct x64_Bytecode {
  u16 length;
  u16 capacity;
  x64_Instruction *buffer;
} x64_Bytecode;

x64_Bytecode x64_bytecode_create();
void x64_bytecode_destroy(x64_Bytecode *restrict bc);

void x64_bytecode_append_ret(x64_Bytecode *restrict bc);
void x64_bytecode_append_call(x64_Bytecode *restrict bc, x64_Operand A);

void x64_bytecode_append_push(x64_Bytecode *restrict bc, x64_Operand A);
void x64_bytecode_prepend_push(x64_Bytecode *restrict bc, x64_Operand A);

void x64_bytecode_append_pop(x64_Bytecode *restrict bc, x64_Operand A);

void x64_bytecode_append_mov(x64_Bytecode *restrict bc,
                             x64_Operand A,
                             x64_Operand B);
void x64_bytecode_prepend_mov(x64_Bytecode *restrict bc,
                              x64_Operand A,
                              x64_Operand B);

void x64_bytecode_append_neg(x64_Bytecode *restrict bc, x64_Operand A);

void x64_bytecode_append_add(x64_Bytecode *restrict bc,
                             x64_Operand A,
                             x64_Operand B);

void x64_bytecode_append_sub(x64_Bytecode *restrict bc,
                             x64_Operand A,
                             x64_Operand B);
void x64_bytecode_prepend_sub(x64_Bytecode *restrict bc,
                              x64_Operand A,
                              x64_Operand B);

void x64_bytecode_append_imul(x64_Bytecode *restrict bc, x64_Operand A);

void x64_bytecode_append_idiv(x64_Bytecode *restrict bc, x64_Operand A);

void x64_bytecode_emit(x64_Bytecode *restrict bc,
                       String *restrict buffer,
                       Context *restrict context);
#endif // !EXP_BACKEND_X64_BYTECODE_H