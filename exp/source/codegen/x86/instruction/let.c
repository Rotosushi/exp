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

#include "codegen/x86/instruction/let.h"
#include "codegen/x86/intrinsics/load.h"
#include "support/unreachable.h"

void x86_codegen_let(Instruction I,
                     u64         block_index,
                     x86_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = x86_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *B = x86_context_allocation_of(context, I.B_data.ssa);
        x86_context_allocate_from_active(context, local, B, block_index);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        Value          *value =
            context_constants_at(context->context, I.B_data.constant);
        x86_codegen_load_allocation_from_value(A, value, block_index, context);
        break;
    }

    case OPERAND_KIND_I64: {
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_immediate(I.B_data.i64_)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
