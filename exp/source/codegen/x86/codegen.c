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
#include "codegen/x86/instruction/load.h"
#include "codegen/x86/instruction/mod.h"
#include "codegen/x86/instruction/mul.h"
#include "codegen/x86/instruction/neg.h"
#include "codegen/x86/instruction/ret.h"
#include "codegen/x86/instruction/sub.h"
#include "support/message.h"
#include "support/unreachable.h"

/*
 * #TODO:
 *  a popular replacement for this handrolled backend is to generate
 *  assembly based on some form of x86-64 specification language.
 *  which if done well, can allow other backends to be written only
 *  by adding a specification of them.
 */

static void x86_codegen_bytecode(x86_Context *x64_context) {
    Bytecode *bc = x86_context_current_bc(x64_context);
    for (u64 idx = 0; idx < bc->length; ++idx) {
        Instruction I = bc->buffer[idx];

        switch (I.opcode) {
        case OPCODE_RET: {
            x86_codegen_ret(I, idx, x64_context);
            break;
        }

        case OPCODE_CALL: {
            x86_codegen_call(I, idx, x64_context);
            break;
        }

        case OPCODE_DOT: {
            x86_codegen_dot(I, idx, x64_context);
            break;
        }

        case OPCODE_LOAD: {
            x86_codegen_load(I, idx, x64_context);
            break;
        }

        case OPCODE_NEG: {
            x86_codegen_negate(I, idx, x64_context);
            break;
        }

        case OPCODE_ADD: {
            x86_codegen_add(I, idx, x64_context);
            break;
        }

        case OPCODE_SUB: {
            x86_codegen_sub(I, idx, x64_context);
            break;
        }

        case OPCODE_MUL: {
            x86_codegen_mul(I, idx, x64_context);
            break;
        }

        case OPCODE_DIV: {
            x86_codegen_div(I, idx, x64_context);
            break;
        }

        case OPCODE_MOD: {
            x86_codegen_mod(I, idx, x64_context);
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }
}

static void x86_codegen_allocate_stack_space(x86_Context *x64_context) {
    i64 stack_size = x86_context_stack_size(x64_context);
    if (i64_in_range_i16(stack_size)) {
        x86_context_prepend(x64_context,
                            x86_sub(x86_operand_gpr(X86_GPR_RSP),
                                    x86_operand_immediate((i16)stack_size)));
    } else {
        Operand operand = context_constants_append(
            x64_context->context, value_create_i64(stack_size));
        assert(operand.kind == OPERAND_KIND_CONSTANT);
        x86_context_prepend(
            x64_context,
            x86_sub(x86_operand_gpr(X86_GPR_RSP),
                    x86_operand_constant(operand.data.constant)));
    }
}

static void x86_codegen_prepend_function_header(x86_Context *x64_context) {
    if (x86_context_uses_stack(x64_context)) {
        x86_codegen_allocate_stack_space(x64_context);
    }

    x86_context_prepend(
        x64_context,
        x86_mov(x86_operand_gpr(X86_GPR_RBP), x86_operand_gpr(X86_GPR_RSP)));
    x86_context_prepend(x64_context, x86_push(x86_operand_gpr(X86_GPR_RBP)));
}

static void x86_codegen_function(x86_Context *x64_context) {
    x86_codegen_bytecode(x64_context);
    x86_codegen_prepend_function_header(x64_context);
}

static void x86_codegen_symbol(Symbol *symbol, x86_Context *x64_context) {
    if (context_trace(x64_context->context)) {
        trace(SV("x64_codegen_symbol:"), stdout);
        trace(symbol->name, stdout);
    }
    StringView name = symbol->name;

    switch (symbol->kind) {
    case SYMBOL_KIND_UNDEFINED: {
        break;
    }

    case SYMBOL_KIND_FUNCTION: {
        x86_context_enter_function(x64_context, name);
        x86_codegen_function(x64_context);
        if (context_trace(x64_context->context) &&
            context_prolix(x64_context->context)) {
            String buffer = string_create();
            string_append(&buffer, SV("Generated x86-64 function:"));
            string_append(&buffer, name);
            x86_bytecode_emit(
                &x64_context->x64_body->bc, &buffer, x64_context->context);
            trace(string_to_view(&buffer), stdout);
            string_destroy(&buffer);
        }
        x86_context_leave_function(x64_context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

i32 x86_codegen(Context *context) {
    if (context_trace(context)) {
        trace(SV("x64_codegen"), stderr);
        trace(context_source_path(context), stderr);
    }
    x86_Context  x64context = x86_context_create(context);
    SymbolTable *table      = &context->global_symbol_table;

    for (u64 index = 0; index < table->capacity; ++index) {
        Symbol *element = table->elements[index];
        if (element == NULL) { continue; }
        x86_codegen_symbol(element, &x64context);
    }

    x86_emit(&x64context);
    x86_context_destroy(&x64context);
    return 0;
}
