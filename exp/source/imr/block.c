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
#include <stddef.h>

#include "env/context.h"
#include "imr/block.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/unreachable.h"

void block_initialize(Block *block) {
    assert(block != nullptr);
    block->length   = 0;
    block->capacity = 0;
    block->buffer   = nullptr;
}

void block_terminate(Block *bytecode) {
    assert(bytecode != nullptr);
    bytecode->length   = 0;
    bytecode->capacity = 0;
    deallocate(bytecode->buffer);
    bytecode->buffer = nullptr;
}

static bool bytecode_full(Block *bytecode) {
    return bytecode->capacity <= (bytecode->length + 1);
}

static void bytecode_grow(Block *bytecode) {
    Growth32 g = array_growth_u32(bytecode->capacity, sizeof(Instruction));
    bytecode->buffer   = reallocate(bytecode->buffer, g.alloc_size);
    bytecode->capacity = g.new_capacity;
}

void block_append(Block *bytecode, Instruction I) {
    if (bytecode_full(bytecode)) { bytecode_grow(bytecode); }

    bytecode->buffer[bytecode->length] = I;
    bytecode->length += 1;
}

static void
print_B(String *buffer, StringView mnemonic, Instruction I, Context *context) {
    string_append(buffer, mnemonic);
    string_append(buffer, SV(" "));
    print_operand(buffer, I.B_kind, I.B_data, context);
}

static void
print_AB(String *buffer, StringView mnemonic, Instruction I, Context *context) {
    string_append(buffer, mnemonic);
    string_append(buffer, SV(" "));
    print_operand(buffer, I.A_kind, I.A_data, context);
    string_append(buffer, SV(", "));
    print_operand(buffer, I.B_kind, I.B_data, context);
}

static void print_ABC(String *buffer,
                      StringView mnemonic,
                      Instruction I,
                      Context *context) {
    string_append(buffer, mnemonic);
    string_append(buffer, SV(" "));
    print_operand(buffer, I.A_kind, I.A_data, context);
    string_append(buffer, SV(", "));
    print_operand(buffer, I.B_kind, I.B_data, context);
    string_append(buffer, SV(", "));
    print_operand(buffer, I.C_kind, I.C_data, context);
}

static void print_instruction(String *buffer, Instruction I, Context *context) {
    switch (I.opcode) {
    case OPCODE_RETURN:   print_B(buffer, SV("ret"), I, context); break;
    case OPCODE_CALL:     print_ABC(buffer, SV("call"), I, context); break;
    case OPCODE_DOT:      print_ABC(buffer, SV("dot"), I, context); break;
    case OPCODE_LOAD:     print_AB(buffer, SV("load"), I, context); break;
    case OPCODE_NEGATE:   print_AB(buffer, SV("neg"), I, context); break;
    case OPCODE_ADD:      print_ABC(buffer, SV("add"), I, context); break;
    case OPCODE_SUBTRACT: print_ABC(buffer, SV("sub"), I, context); break;
    case OPCODE_MULTIPLY: print_ABC(buffer, SV("mul"), I, context); break;
    case OPCODE_DIVIDE:   print_ABC(buffer, SV("div"), I, context); break;
    case OPCODE_MODULUS:  print_ABC(buffer, SV("mod"), I, context); break;

    default: EXP_UNREACHABLE();
    }
}

void print_block(String *buffer, Block const *bc, Context *context) {
    for (u32 i = 0; i < bc->length; ++i) {
        string_append(buffer, SV("\t"));
        string_append_u64(buffer, i);
        string_append(buffer, SV(": "));
        print_instruction(buffer, bc->buffer[i], context);
        string_append(buffer, SV("\n"));
    }
}
