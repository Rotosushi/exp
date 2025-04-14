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

#include "codegen/x86/instruction/ret.h"
#include "codegen/x86/intrinsics/copy.h"
#include "codegen/x86/intrinsics/load.h"
#include "support/message.h"
#include "support/panic.h"
#include "support/unreachable.h"

void x86_codegen_ret(Instruction I,
                     u64         block_index,
                     x86_Context *restrict context) {
    x86_Function *body = x86_context_current_x86_body(context);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_return: ssa"), stdout);
        }
        x86_Allocation *B = x86_context_allocation_of(context, I.B_data.ssa);
        if (x86_allocation_location_eq(B, body->result->location)) { break; }
        x86_codegen_copy_allocation(body->result, B, block_index, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_return: constant"), stdout);
        }
        Value *value =
            context_constants_at(context->context, I.B_data.constant);
        x86_codegen_load_allocation_from_value(
            body->result, value, block_index, context);
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_return: i64"), stdout);
        }
        x86_context_append(context,
                           x64_mov(x86_operand_alloc(body->result),
                                   x86_operand_immediate(I.B_data.i64_)));
        break;
    }

    case OPERAND_KIND_LABEL: {
        /*
         * #NOTE #TODO #FEATURE eventually we will add support for
         * global constants (global variables are in limbo until
         * proven vital). When these exist, it will be possible to
         * access them via OPRFMT_LABEL operands. Since we do not
         * have them yet, this case is effecively unreachable.
         * (right now OPRFMT_LABEL is used exclusively for global
         *  functions. which are global constants.)
         */
        PANIC("#TODO");
        break;
    }

    default: EXP_UNREACHABLE();
    }

    x86_context_append(
        context,
        x64_mov(x86_operand_gpr(X86_GPR_RSP), x86_operand_gpr(X86_GPR_RBP)));
    x86_context_append(context, x64_pop(x86_operand_gpr(X86_GPR_RBP)));
    x86_context_append(context, x64_ret());
}
