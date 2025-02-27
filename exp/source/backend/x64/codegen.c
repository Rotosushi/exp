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
#include "backend/x64/codegen/addition.h"
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
 * #TODO #CLEANUP
 *  so, this code is rather difficult to read and modify.
 *  as it is all handrolled nested switch statements.
 *  The best thing for now is going to be factoring out
 *  each case into it's own function. This is going to
 *  create a lot more functions, but hopefully it will make
 *  them all easier to read, and hopefully maintain.
 *
 *  a popular replacement for this handrolling is to generate
 *  these switches based on some form of x64 specification language.
 */

static void x64_codegen_bytecode(x64_Context *restrict context) {
    Bytecode *bc = current_bc(context);
    for (u64 idx = 0; idx < bc->length; ++idx) {
        Instruction I = bc->buffer[idx];

        switch (I.opcode) {
        case OPC_RET: {
            x64_codegen_return(I, idx, context);
            break;
        }

        case OPC_CALL: {
            x64_codegen_call(I, idx, context);
            break;
        }

        case OPC_DOT: {
            x64_codegen_dot(I, idx, context);
            break;
        }

        case OPC_LOAD: {
            x64_codegen_load(I, idx, context);
            break;
        }

        case OPC_NEG: {
            x64_codegen_negate(I, idx, context);
            break;
        }

        case OPC_ADD: {
            x64_codegen_addition(I, idx, context);
            break;
        }

        case OPC_SUB: {
            x64_codegen_subtract(I, idx, context);
            break;
        }

        case OPC_MUL: {
            x64_codegen_multiply(I, idx, context);
            break;
        }

        case OPC_DIV: {
            x64_codegen_divide(I, idx, context);
            break;
        }

        case OPC_MOD: {
            x64_codegen_modulus(I, idx, context);
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }
}

static void x64_codegen_function_header(x64_Context *restrict context) {
    if (x64_context_uses_stack(context)) {
        x64_context_prepend(
            context,
            x64_sub(x64_operand_gpr(X64GPR_RSP),
                    x64_operand_immediate(x64_context_stack_size(context))));
    }
    x64_context_prepend(
        context,
        x64_mov(x64_operand_gpr(X64GPR_RBP), x64_operand_gpr(X64GPR_RSP)));
    x64_context_prepend(context, x64_push(x64_operand_gpr(X64GPR_RBP)));
}

static void x64_codegen_function(x64_Context *restrict context) {
    x64_codegen_bytecode(context);
    x64_codegen_function_header(context);
}

static void x64_codegen_ste(SymbolTableElement *restrict ste,
                            x64_Context *restrict context) {
    StringView name = ste->name;

    switch (ste->kind) {
    case STE_UNDEFINED: {
        // #TODO this should lower to a forward declaration
        break;
    }

    case STE_FUNCTION: {
        x64_context_enter_function(context, name);
        x64_codegen_function(context);
        x64_context_leave_function(context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

void x64_codegen(Context *restrict context) {
    x64_Context x64context = x64_context_create(context);

    SymbolTableIterator iter = context_global_symbol_table_iterator(context);

    while (!symbol_table_iterator_done(&iter)) {
        x64_codegen_ste(iter.element, &x64context);

        symbol_table_iterator_next(&iter);
    }

    x64_emit(&x64context);
    x64_context_destroy(&x64context);
}
