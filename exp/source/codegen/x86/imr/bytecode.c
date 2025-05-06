/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "codegen/x86/imr/bytecode.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void x86_bytecode_create(x86_Bytecode *restrict bc) {
    exp_assert(bc != NULL);
    bc->length   = 0;
    bc->capacity = 0;
    bc->buffer   = NULL;
}

void x86_bytecode_destroy(x86_Bytecode *restrict bc) {
    exp_assert(bc != NULL);
    deallocate(bc->buffer);
    x86_bytecode_create(bc);
}

static bool x86_bytecode_full(x86_Bytecode *restrict bc) {
    return (bc->length + 1) >= bc->capacity;
}

static void x86_bytecode_grow(x86_Bytecode *restrict bc) {
    Growth_u32 g = array_growth_u32(bc->capacity, sizeof(x86_Instruction));
    bc->buffer   = reallocate(bc->buffer, g.alloc_size);
    bc->capacity = g.new_capacity;
}

u32 x86_bytecode_current_offset(x86_Bytecode *restrict bc) {
    return bc->length;
}

void x86_bytecode_insert(x86_Bytecode *restrict bc,
                         x86_Instruction instruction,
                         u64             offset) {
    exp_assert(bc != NULL);
    exp_assert(offset < bc->capacity);
    exp_assert(offset <= bc->length);
    if (x86_bytecode_full(bc)) { x86_bytecode_grow(bc); }

    for (u32 index = bc->length; index > offset; --index) {
        bc->buffer[index] = bc->buffer[index - 1];
    }

    bc->buffer[offset] = instruction;
    bc->length += 1;
}

void x86_bytecode_append(x86_Bytecode *restrict bc,
                         x86_Instruction instruction) {
    exp_assert(bc != NULL);
    x86_bytecode_insert(bc, instruction, bc->length);
}

void x86_bytecode_prepend(x86_Bytecode *restrict bc,
                          x86_Instruction instruction) {
    exp_assert(bc != NULL);
    x86_bytecode_insert(bc, instruction, 0);
}

void print_x86_bytecode(String *restrict buffer,
                        x86_Bytecode const *restrict bc) {
    exp_assert(buffer != NULL);
    exp_assert(bc != NULL);
    for (u32 index = 0; index < bc->length; ++index) {
        string_append(buffer, SV("\t"));
        print_x86_instruction(buffer, bc->buffer[index]);
        string_append(buffer, SV("\n"));
    }
}
