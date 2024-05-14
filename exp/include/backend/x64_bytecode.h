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

#include "backend/x64_instruction.h"

typedef struct X64Bytecode {
  u16 length;
  u16 capacity;
  X64Instruction *buffer;
} X64Bytecode;

X64Bytecode x64bytecode_create();
void x64bytecode_destroy(X64Bytecode *restrict bc);

void x64bytecode_emit_ret(X64Bytecode *restrict bc);

void x64bytecode_emit_push(X64Bytecode *restrict bc, X64Operand A);
void x64bytecode_emit_pop(X64Bytecode *restrict bc, X64Operand A);

void x64bytecode_emit_mov(X64Bytecode *restrict bc, X64Operand A, X64Operand B);

void x64bytecode_emit_add(X64Bytecode *restrict bc, X64Operand A, X64Operand B);
void x64bytecode_emit_sub(X64Bytecode *restrict bc, X64Operand A, X64Operand B);
void x64bytecode_emit_imul(X64Bytecode *restrict bc, X64Operand A);
void x64bytecode_emit_idiv(X64Bytecode *restrict bc, X64Operand A);
#endif // !EXP_BACKEND_X64_BYTECODE_H