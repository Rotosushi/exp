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
#include <assert.h>

#include "codegen/x86/imr/bytecode.h"
#include "support/allocation.h"
#include "support/array_growth.h"

x86_Bytecode x86_bytecode_create() {
    x86_Bytecode bc = {.length = 0, .capacity = 0, .buffer = NULL};
    return bc;
}

void x86_bytecode_destroy(x86_Bytecode *restrict bc) {
    assert(bc != NULL);
    bc->length   = 0;
    bc->capacity = 0;
    deallocate(bc->buffer);
    bc->buffer = NULL;
}

static bool x86_bytecode_full(x86_Bytecode *restrict bc) {
    return (bc->length + 1) >= bc->capacity;
}

static void x86_bytecode_grow(x86_Bytecode *restrict bc) {
    Growth_u64 g = array_growth_u64(bc->capacity, sizeof(x86_Instruction));
    bc->buffer   = reallocate(bc->buffer, g.alloc_size);
    bc->capacity = g.new_capacity;
}

u64 x86_bytecode_current_offset(x86_Bytecode *restrict bc) {
    return bc->length;
}

void x86_bytecode_insert(x86_Bytecode *restrict bc,
                         x86_Instruction I,
                         u64             offset) {
    assert(bc != NULL);
    assert(offset < bc->capacity);
    assert(offset <= bc->length);
    if (x86_bytecode_full(bc)) { x86_bytecode_grow(bc); }

    for (u64 i = bc->length; i > offset; --i) {
        bc->buffer[i] = bc->buffer[i - 1];
    }

    bc->buffer[offset] = I;
    bc->length += 1;
}

void x86_bytecode_append(x86_Bytecode *restrict bc, x86_Instruction I) {
    assert(bc != NULL);
    if (x86_bytecode_full(bc)) { x86_bytecode_grow(bc); }

    bc->buffer[bc->length] = I;
    bc->length += 1;
}

void x86_bytecode_prepend(x86_Bytecode *restrict bc, x86_Instruction I) {
    assert(bc != NULL);
    if (x86_bytecode_full(bc)) { x86_bytecode_grow(bc); }

    // move all instructions forward one location
    for (u64 idx = bc->length; idx > 0; --idx) {
        bc->buffer[idx] = bc->buffer[idx - 1];
    }

    bc->buffer[0] = I;
    bc->length += 1;
}

void x86_bytecode_emit(x86_Bytecode *restrict bc,
                       String *restrict buffer,
                       Context *restrict context) {
    for (u16 i = 0; i < bc->length; ++i) {
        string_append(buffer, SV("\t"));
        x86_instruction_emit(bc->buffer[i], buffer, context);
        string_append(buffer, SV("\n"));
    }
}
