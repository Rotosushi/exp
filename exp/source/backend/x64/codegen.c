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

#include "backend/x64/codegen.h"
#include "backend/x64/codegen/add.h"
#include "backend/x64/codegen/call.h"
#include "backend/x64/codegen/divide.h"
#include "backend/x64/codegen/dot.h"
#include "backend/x64/codegen/load.h"
#include "backend/x64/codegen/modulus.h"
#include "backend/x64/codegen/multiply.h"
#include "backend/x64/codegen/negate.h"
#include "backend/x64/codegen/return.h"
#include "backend/x64/codegen/subtract.h"
#include "backend/x64/context.h"
#include "backend/x64/emit.h"
#include "utility/unreachable.h"

/*
 * #TODO:
 *  a popular replacement for this handrolled backend is to generate
 *  assembly based on some form of x86-64 specification language.
 *  which if done well, can allow other backends to be written only
 *  by adding a specification of them.
 */

static void x64_codegen_bytecode(x64_Context *x64_context) {
    Block *block = x64_context_current_block(x64_context);
    for (u64 idx = 0; idx < block->length; ++idx) {
        Instruction instruction = block->buffer[idx];

        switch (instruction.opcode) {
        case OPCODE_RETURN: {
            x64_codegen_return(instruction, idx, x64_context);
            break;
        }

        case OPCODE_CALL: {
            x64_codegen_call(instruction, idx, x64_context);
            break;
        }

        case OPCODE_DOT: {
            x64_codegen_dot(instruction, idx, x64_context);
            break;
        }

        case OPCODE_LOAD: {
            x64_codegen_load(instruction, idx, x64_context);
            break;
        }

        case OPCODE_NEGATE: {
            x64_codegen_negate(instruction, idx, x64_context);
            break;
        }

        case OPCODE_ADD: {
            x64_codegen_add(instruction, idx, x64_context);
            break;
        }

        case OPCODE_SUBTRACT: {
            x64_codegen_subtract(instruction, idx, x64_context);
            break;
        }

        case OPCODE_MULTIPLY: {
            x64_codegen_multiply(instruction, idx, x64_context);
            break;
        }

        case OPCODE_DIVIDE: {
            x64_codegen_divide(instruction, idx, x64_context);
            break;
        }

        case OPCODE_MODULUS: {
            x64_codegen_modulus(instruction, idx, x64_context);
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }
}

static void x64_codegen_allocate_stack_space(x64_Context *x64_context) {
    i64 stack_size = x64_context_stack_size(x64_context);
    if (i64_in_range_i16(stack_size)) {
        x64_context_prepend(x64_context,
                            x64_sub(x64_operand_gpr(X64_GPR_RSP),
                                    x64_operand_immediate((i16)stack_size)));
    } else {
        Operand operand = context_constants_append(
            x64_context->context, value_create_i64(stack_size));
        assert(operand.kind == OPERAND_KIND_CONSTANT);
        x64_context_prepend(
            x64_context,
            x64_sub(x64_operand_gpr(X64_GPR_RSP),
                    x64_operand_constant(operand.data.constant)));
    }
}

static void x64_codegen_prepend_function_header(x64_Context *x64_context) {
    if (x64_context_uses_stack(x64_context)) {
        x64_codegen_allocate_stack_space(x64_context);
    }

    x64_context_prepend(
        x64_context,
        x64_mov(x64_operand_gpr(X64_GPR_RBP), x64_operand_gpr(X64_GPR_RSP)));
    x64_context_prepend(x64_context, x64_push(x64_operand_gpr(X64_GPR_RBP)));
}

static void x64_codegen_function(x64_Context *x64_context) {
    x64_codegen_bytecode(x64_context);
    x64_codegen_prepend_function_header(x64_context);
}

static void x64_codegen_symbol(x64_Context *x64_context, Symbol *symbol) {
    FunctionBody *body         = &symbol->function_body;
    x64_Symbol *x64_symbol     = x64_context_symbol(x64_context, symbol->name);
    x64_FunctionBody *x64_body = &x64_symbol->body;
    x64_function_body_create(x64_body, body, x64_context);
    x64_context_enter_function(x64_context, body, x64_body);
    x64_codegen_function(x64_context);
    x64_context_leave_function(x64_context);
}

void x64_codegen(Context *context) {
    x64_Context x64_context = x64_context_create(context);

    SymbolTable *symbol_table = &context->symbol_table;
    for (u64 i = 0; i < symbol_table->capacity; ++i) {
        Symbol *symbol = symbol_table->elements[i];
        if (symbol == nullptr) { continue; }
        x64_codegen_symbol(&x64_context, symbol);
    }

    x64_emit(&x64_context);
    x64_context_destroy(&x64_context);
}
