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

#include "codegen/x64/instruction/subtract.h"
#include "support/assert.h"
#include "support/unreachable.h"

static void x64_codegen_subtract_ssa(Instruction I,
                                     u64 block_index,
                                     LocalVariable *local,
                                     x64_Context *context) {
    x64_Allocation *B = x64_context_allocation_of(context, I.B_data.ssa);
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);
        // #NOTE since subtraction is not commutative we have to allocate A from
        // B regardless of which of B or C is in a register.
        if ((B->location.kind == LOCATION_GPR) ||
            (C->location.kind == LOCATION_GPR)) {
            x64_Allocation *A = x64_context_allocate_from_active(
                context, local, B, block_index);

            x64_context_append(
                context, x64_sub(x64_operand_alloc(A), x64_operand_alloc(C)));
            return;
        }

        x64_Allocation *A = x64_context_allocate_to_any_gpr(context, local);

        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A), x64_operand_alloc(B)));

        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A), x64_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, B, block_index);

        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A),
                                   x64_operand_immediate(I.C_data.i64_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, B, block_index);

        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A),
                                   x64_operand_constant(I.C_data.constant)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x64_codegen_subtract_immediate(Instruction I,
                                           u64 block_index,
                                           LocalVariable *local,
                                           x64_Context *context) {
    switch (I.C_kind) {
        /*
         * #NOTE: there is no x64 sub instruction which takes an
         * immediate value on the lhs. so we have to move the
         * value of B into a gpr and allocate A there.
         * Then we can emit the sub instruction.
         */
    case OPERAND_KIND_SSA: {
        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);

        x64_Allocation *A = x64_context_allocate_to_any_gpr(context, local);
        exp_assert_debug(A->location.kind == LOCATION_GPR);
        x86_64_GPR gpr = A->location.gpr;
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_immediate(I.B_data.i64_)));

        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A), x64_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_immediate(I.B_data.i64_)));
        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A),
                                   x64_operand_immediate(I.C_data.i64_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_immediate(I.B_data.i64_)));
        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A),
                                   x64_operand_constant(I.C_data.constant)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x64_codegen_subtract_constant(Instruction I,
                                   u64 block_index,
                                   LocalVariable *local,
                                   x64_Context *context) {
    switch (I.C_kind) {
        /*
         * #NOTE: there is no x64 sub instruction which takes an
         *  constant value on the lhs. so we have to move the
         *  value of B into a gpr and allocate A there.
         *  Then we can emit the sub instruction.
         */
    case OPERAND_KIND_SSA: {
        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);

        x64_Allocation *A = x64_context_allocate_to_any_gpr(context, local);
        exp_assert_debug(A->location.kind == LOCATION_GPR);
        x86_64_GPR gpr = A->location.gpr;
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_constant(I.B_data.constant)));

        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A), x64_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_constant(I.B_data.constant)));
        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A),
                                   x64_operand_immediate(I.C_data.i64_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_constant(I.B_data.constant)));
        x64_context_append(context,
                           x64_sub(x64_operand_alloc(A),
                                   x64_operand_constant(I.C_data.constant)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x64_codegen_subtract(Instruction I,
                          u64 block_index,
                          x64_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = x64_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x64_codegen_subtract_ssa(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_I64: {
        x64_codegen_subtract_immediate(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_codegen_subtract_constant(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
