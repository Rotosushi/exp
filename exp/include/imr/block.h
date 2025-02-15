// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_BLOCK_H
#define EXP_IMR_BLOCK_H

#include "imr/instruction.h"

/**
 * @brief represents a section of instructions.
 */
typedef struct Block {
    u32 length;
    u32 capacity;
    Instruction *buffer;
} Block;

void block_initialize(Block *block);
void block_terminate(Block *block);

void block_append(Block *block, Instruction instruction);

struct Context;
void print_block(String *buffer, Block const *block, struct Context *context);
#endif // !EXP_IMR_BLOCK_H
