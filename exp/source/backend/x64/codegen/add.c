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

#include "backend/x64/codegen/add.h"
#include "intrinsics/size_of.h"
#include "utility/unreachable.h"

static void x64_codegen_add_ssa(Instruction I,
                                u64 block_index,
                                LocalVariable *local,
                                x64_Context *context) {
    x64_Allocation *B = x64_context_allocation_of(context, I.B_data.ssa);
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);
        // if B or C is in a gpr we use it as the allocation point of A
        // and the destination operand of the x64 add instruction.
        // this is to try and keep the result, A, in a register.
        if (B->location.kind == LOCATION_GPR) {
            x64_Allocation *A = x64_context_allocate_from_active(
                context, local, B, block_index);
            x64_context_append(context,
                               x64_add(x64_operand_location(A->location),
                                       x64_operand_location(C->location)));
            return;
        }

        if (C->location.kind == LOCATION_GPR) {
            x64_Allocation *A = x64_context_allocate_from_active(
                context, local, C, block_index);
            x64_context_append(context,
                               x64_add(x64_operand_location(A->location),
                                       x64_operand_location(B->location)));
            return;
        }

        // since B and C are memory operands we have to move B or C
        // to a reg and then add.
        x64_GPR gpr = x64_context_aquire_any_gpr(
            context, block_index, size_of(local->type));
        x64_Allocation *A =
            x64_context_allocate_to_gpr(context, local, gpr, block_index);

        // we use the huristic of longest lifetime to choose
        // which of B and C to move into A's gpr.
        if (B->lifetime.last_use <= C->lifetime.last_use) {
            x64_context_append(context,
                               x64_mov(x64_operand_location(A->location),
                                       x64_operand_location(C->location)));
            x64_context_append(context,
                               x64_add(x64_operand_location(A->location),
                                       x64_operand_location(B->location)));
        } else {
            x64_context_append(context,
                               x64_mov(x64_operand_location(A->location),
                                       x64_operand_location(B->location)));
            x64_context_append(context,
                               x64_add(x64_operand_location(A->location),
                                       x64_operand_location(C->location)));
        }
        break;
    }

    case OPERAND_KIND_I32: {
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, B, block_index);

        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_immediate(I.C_data.i32_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, B, block_index);

        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_constant(I.C_data.constant)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x64_codegen_add_immediate(Instruction I,
                                      u64 block_index,
                                      LocalVariable *local,
                                      x64_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, C, block_index);

        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_immediate(I.B_data.i32_)));
        break;
    }

    case OPERAND_KIND_I32: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_location(A->location),
                                   x64_operand_immediate(I.B_data.i32_)));
        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_immediate(I.C_data.i32_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_location(A->location),
                                   x64_operand_constant(I.B_data.constant)));

        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_immediate(I.C_data.i32_)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x64_codegen_add_constant(Instruction I,
                                     u64 block_index,
                                     LocalVariable *local,
                                     x64_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, C, block_index);

        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_constant(I.B_data.constant)));
        break;
    }

    case OPERAND_KIND_I32: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_location(A->location),
                                   x64_operand_constant(I.B_data.constant)));

        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_immediate(I.C_data.i32_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_location(A->location),
                                   x64_operand_constant(I.B_data.constant)));

        x64_context_append(context,
                           x64_add(x64_operand_location(A->location),
                                   x64_operand_constant(I.C_data.constant)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x64_codegen_add(Instruction I,
                     u64 block_index,
                     x64_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = x64_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x64_codegen_add_ssa(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_I32: {
        x64_codegen_add_immediate(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_codegen_add_constant(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
