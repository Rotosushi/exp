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

#include "codegen/x86/codegen.h"
#include "codegen/x86/emit.h"
#include "codegen/x86/env/context.h"
#include "codegen/x86/instruction/add.h"
#include "codegen/x86/instruction/call.h"
#include "codegen/x86/instruction/div.h"
#include "codegen/x86/instruction/dot.h"
#include "codegen/x86/instruction/let.h"
#include "codegen/x86/instruction/mod.h"
#include "codegen/x86/instruction/mul.h"
#include "codegen/x86/instruction/neg.h"
#include "codegen/x86/instruction/ret.h"
#include "codegen/x86/instruction/sub.h"
#include "support/unreachable.h"

/*
 * #TODO:
 *  a popular replacement for this handrolled backend is to generate
 *  assembly based on some form of x86-64 specification language.
 *  which if done well, can allow other backends to be written only
 *  by adding a specification of them. this works in LLVM via TableGen,
 *  Which to my understanding generates a generator. and the machanism
 *  for generation is some form of Graph Covering.
 */

static void x86_codegen_bytecode(x86_Context *x86_context) {
    Bytecode const *bc = x86_context_current_bc(x86_context);
    for (u32 idx = 0; idx < bc->length; ++idx) {
        Instruction I = bc->buffer[idx];

        switch (I.opcode) {
        case OPCODE_RET: {
            x86_codegen_ret(I, idx, x86_context);
            break;
        }

        case OPCODE_CALL: {
            x86_codegen_call(I, idx, x86_context);
            break;
        }

        case OPCODE_DOT: {
            x86_codegen_dot(I, idx, x86_context);
            break;
        }

        case OPCODE_LET: {
            x86_codegen_let(I, idx, x86_context);
            break;
        }

        case OPCODE_NEG: {
            x86_codegen_neg(I, idx, x86_context);
            break;
        }

        case OPCODE_ADD: {
            x86_codegen_add(I, idx, x86_context);
            break;
        }

        case OPCODE_SUB: {
            x86_codegen_sub(I, idx, x86_context);
            break;
        }

        case OPCODE_MUL: {
            x86_codegen_mul(I, idx, x86_context);
            break;
        }

        case OPCODE_DIV: {
            x86_codegen_div(I, idx, x86_context);
            break;
        }

        case OPCODE_MOD: {
            x86_codegen_mod(I, idx, x86_context);
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }
}

static void x86_codegen_allocate_stack_space(x86_Context *x86_context) {
    i64 stack_size = x86_context_stack_size(x86_context);
    x86_context_prepend(
        x86_context,
        x86_sub(x86_operand_gpr(X86_GPR_RSP), x86_operand_i64(stack_size)));
}

static void x86_codegen_prepend_function_header(x86_Context *x86_context) {
    if (x86_context_uses_stack(x86_context)) {
        x86_codegen_allocate_stack_space(x86_context);
    }

    x86_context_prepend(
        x86_context,
        x86_mov(x86_operand_gpr(X86_GPR_RBP), x86_operand_gpr(X86_GPR_RSP)));
    x86_context_prepend(x86_context, x86_push(x86_operand_gpr(X86_GPR_RBP)));
}

static void x86_codegen_function(x86_Context *x86_context) {
    x86_codegen_bytecode(x86_context);
    x86_codegen_prepend_function_header(x86_context);
}

void x86_codegen_symbol(Symbol *symbol, x86_Context *x86_context) {
    assert(symbol->value->kind == VALUE_KIND_FUNCTION);
    Function const *function = &symbol->value->function;
    x86_context_enter_function(x86_context, function);
    x86_codegen_function(x86_context);
    x86_context_leave_function(x86_context);
}

i32 x86_codegen(Context *context) {
    x86_Context  x86_context = x86_context_create(context);
    SymbolTable *table       = &context->global_symbol_table;

    for (u64 index = 0; index < table->capacity; ++index) {
        Symbol *element = table->elements[index];
        if (element == NULL) { continue; }
        x86_codegen_symbol(element, &x86_context);
    }

    x86_emit(&x86_context);
    x86_context_destroy(&x86_context);
    return 0;
}
