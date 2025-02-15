/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "imr/block.h"
#include "env/context.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

void block_initialize(Block *block) {
    EXP_ASSERT(block != nullptr);
    block->length   = 0;
    block->capacity = 0;
    block->buffer   = nullptr;
}

void block_terminate(Block *bytecode) {
    EXP_ASSERT(bytecode != nullptr);
    bytecode->length   = 0;
    bytecode->capacity = 0;
    deallocate(bytecode->buffer);
    bytecode->buffer = nullptr;
}

static bool bytecode_full(Block *bytecode) {
    EXP_ASSERT(bytecode != nullptr);
    return bytecode->capacity <= (bytecode->length + 1);
}

static void bytecode_grow(Block *bytecode) {
    EXP_ASSERT(bytecode != nullptr);
    Growth32 g = array_growth_u32(bytecode->capacity, sizeof(Instruction));
    bytecode->buffer   = reallocate(bytecode->buffer, g.alloc_size);
    bytecode->capacity = g.new_capacity;
}

void block_append(Block *bytecode, Instruction I) {
    EXP_ASSERT(bytecode != nullptr);
    if (bytecode_full(bytecode)) { bytecode_grow(bytecode); }

    bytecode->buffer[bytecode->length] = I;
    bytecode->length += 1;
}

static void print_B(String *buffer, StringView mnemonic, Instruction I,
                    Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
    string_append(buffer, mnemonic);
    string_append(buffer, SV(" "));
    print_operand(buffer, I.B_kind, I.B_data, context);
}

static void print_AB(String *buffer, StringView mnemonic, Instruction I,
                     Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
    string_append(buffer, mnemonic);
    string_append(buffer, SV(" "));
    print_operand(buffer, I.A_kind, I.A_data, context);
    string_append(buffer, SV(", "));
    print_operand(buffer, I.B_kind, I.B_data, context);
}

static void print_ABC(String *buffer, StringView mnemonic, Instruction I,
                      Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
    string_append(buffer, mnemonic);
    string_append(buffer, SV(" "));
    print_operand(buffer, I.A_kind, I.A_data, context);
    string_append(buffer, SV(", "));
    print_operand(buffer, I.B_kind, I.B_data, context);
    string_append(buffer, SV(", "));
    print_operand(buffer, I.C_kind, I.C_data, context);
}

static void print_instruction(String *buffer, Instruction I, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
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

void print_block(String *buffer, Block const *bytecode, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(bytecode != nullptr);
    EXP_ASSERT(context != nullptr);
    for (u32 i = 0; i < bytecode->length; ++i) {
        string_append(buffer, SV("\t"));
        string_append_u64(buffer, i);
        string_append(buffer, SV(": "));
        print_instruction(buffer, bytecode->buffer[i], context);
        string_append(buffer, SV("\n"));
    }
}
