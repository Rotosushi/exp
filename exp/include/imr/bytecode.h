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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_IMR_BYTECODE_H
#define EXP_IMR_BYTECODE_H
#include "imr/instruction.h"

/**
 * @brief represents a section of instructions.
 */
typedef struct Bytecode {
  u64 length;
  u64 capacity;
  Instruction *buffer;
} Bytecode;

Bytecode bytecode_create();

void bytecode_destroy(Bytecode *restrict bc);

void bytecode_emit_load_immediate(Bytecode *restrict bc, u16 A, u32 I);

void bytecode_emit_neg(Bytecode *restrict bc, u16 A, u16 B);

void bytecode_emit_add(Bytecode *restrict bc, u16 A, u16 B, u16 C);

void bytecode_emit_sub(Bytecode *restrict bc, u16 A, u16 B, u16 C);

void bytecode_emit_mul(Bytecode *restrict bc, u16 A, u16 B, u16 C);

void bytecode_emit_div(Bytecode *restrict bc, u16 A, u16 B, u16 C);

void bytecode_emit_mod(Bytecode *restrict bc, u16 A, u16 B, u16 C);
#endif // !EXP_IMR_BYTECODE_H