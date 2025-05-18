/**
 * Copyright (C) 2025 Cade Weinberg
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

#include "codegen/x86/function.h"
#include "codegen/x86/imr/function.h"
#include "codegen/x86/instruction/ret.h"
#include "support/unreachable.h"

static void x86_codegen_function(x86_Function *restrict x86_function,
                                 Function const *restrict function,
                                 Context *restrict context);

void print_x86_function(String *restrict buffer,
                        Value const *restrict value,
                        Context *restrict context) {
    Function const *function = &value->function;
    x86_Function    x86_function;
    x86_function_create(&x86_function);
    x86_codegen_function(&x86_function, function, context);
    print_x86_bytecode(buffer, &x86_function.body);
    x86_function_destroy(&x86_function);
}

static void x86_codegen_instruction(Instruction instruction,
                                    u32         block_index,
                                    x86_Function *restrict x86_function,
                                    Context *restrict context);

static void x86_codegen_function(x86_Function *restrict x86_function,
                                 Function const *restrict function,
                                 Context *restrict context) {
    x86_function_setup(x86_function, function);
    x86_function_header(x86_function);
    Bytecode const *body = &function->body;
    for (u32 index = 0; index < body->length; ++index) {
        x86_codegen_instruction(
            body->buffer[index], index, x86_function, context);
    }
}

static void x86_codegen_instruction(Instruction instruction,
                                    u32         block_index,
                                    x86_Function *restrict x86_function,
                                    Context *restrict context) {
    switch (instruction.opcode) {
    case OPCODE_RET:
        x86_codegen_ret(instruction, block_index, x86_function, context);
        break;

    default: EXP_UNREACHABLE();
    }
}
