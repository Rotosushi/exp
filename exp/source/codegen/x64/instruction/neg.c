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

#include "codegen/x64/instruction/neg.h"
#include "support/message.h"
#include "support/unreachable.h"

void x64_codegen_negate(Instruction I,
                        u64         block_index,
                        x64_Context *restrict context) {
    LocalVariable *local = x64_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_negate: ssa"), stdout);
        }
        x64_Allocation *B = x64_context_allocation_of(context, I.B_data.ssa);
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, B, block_index);

        x64_context_append(context, x64_neg(x64_operand_alloc(A)));
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_negate: i64"), stdout);
        }
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_immediate(I.B_data.i64_)));
        x64_context_append(context, x64_neg(x64_operand_alloc(A)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_negate: constant"), stdout);
        }
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_constant(I.B_data.constant)));
        x64_context_append(context, x64_neg(x64_operand_alloc(A)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
