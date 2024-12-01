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
#ifndef EXP_BACKEND_X64_BLOCK_H
#define EXP_BACKEND_X64_BLOCK_H

#include "adt/string.h"
#include "backend/x64/instruction.h"

typedef struct x64_Block {
    u64 length;
    u64 capacity;
    x64_Instruction *buffer;
} x64_Block;

void x64_block_initialize(x64_Block *block);
void x64_block_terminate(x64_Block *block);

u64 x64_block_current_offset(x64_Block *block);

void x64_block_insert(x64_Block *block,
                      x64_Instruction instruction,
                      u64 offset);
void x64_block_prepend(x64_Block *block, x64_Instruction instruction);
void x64_block_append(x64_Block *block, x64_Instruction instruction);

void x64_block_emit(x64_Block *block,
                    String *buffer,
                    struct x64_Context *context);
#endif // !EXP_BACKEND_X64_BLOCK_H
