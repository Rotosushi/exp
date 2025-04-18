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
 *
 * @todo bytecode is a holdover name, and no longer reflects
 * the structure. What is a name which is independant of the
 * underlying implementation? Chunk? Instructions? Block?
 */
typedef struct Bytecode {
    u32          length;
    u32          capacity;
    Instruction *buffer;
} Bytecode;

void bytecode_create(Bytecode *restrict bytecode);
void bytecode_destroy(Bytecode *restrict bytecode);

void bytecode_append(Bytecode *restrict bytecode, Instruction I);

struct Context;
void print_bytecode(String *restrict string,
                    Bytecode const *restrict bc,
                    struct Context *restrict context);
#endif // !EXP_IMR_BYTECODE_H
