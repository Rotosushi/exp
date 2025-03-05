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

#include "codegen/x64/instruction/divide.h"
#include "utility/unreachable.h"

static void x64_codegen_divide_ssa(Instruction I,
                                   u64 block_index,
                                   LocalVariable *local,
                                   x64_Context *context) {
    x64_Allocation *B = x64_context_allocation_of(context, I.B_data.ssa);
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);
        if ((B->location.kind == LOCATION_GPR) &&
            (B->location.gpr == X64_GPR_RAX)) {
            x64_context_allocate_from_active(context, local, B, block_index);

            x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
            x64_context_append(context,
                               x64_mov(x64_operand_gpr(X64_GPR_RDX),
                                       x64_operand_immediate(0)));

            x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
            x64_context_release_gpr(context, X64_GPR_RDX, block_index);
            break;
        }

        if ((C->location.kind == LOCATION_GPR) &&
            (C->location.gpr == X64_GPR_RAX)) {
            x64_context_allocate_to_gpr(
                context, local, X64_GPR_RAX, block_index);

            x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
            x64_context_append(context,
                               x64_mov(x64_operand_gpr(X64_GPR_RDX),
                                       x64_operand_immediate(0)));

            x64_context_reallocate_active(context, C);

            x64_context_append(
                context,
                x64_mov(x64_operand_gpr(X64_GPR_RAX), x64_operand_alloc(B)));

            x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
            x64_context_release_gpr(context, X64_GPR_RDX, block_index);
            break;
        }

        x64_context_allocate_to_gpr(context, local, X64_GPR_RAX, block_index);

        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RAX), x64_operand_alloc(B)));

        x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_IMMEDIATE: {
        x64_context_allocate_to_gpr(context, local, X64_GPR_RAX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RAX), x64_operand_alloc(B)));

        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_GPR gpr = x64_context_aquire_any_gpr(context, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_immediate(I.C_data.immediate)));

        x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_context_allocate_to_gpr(context, local, X64_GPR_RAX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RAX), x64_operand_alloc(B)));

        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_GPR gpr = x64_context_aquire_any_gpr(context, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_constant(I.C_data.constant)));

        x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);

        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x64_codegen_divide_immediate(Instruction I,
                                         u64 block_index,
                                         LocalVariable *local,
                                         x64_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);
        if ((C->location.kind == LOCATION_GPR) &&
            (C->location.gpr == X64_GPR_RAX)) {
            x64_context_reallocate_active(context, C);
        }

        x64_context_allocate_to_gpr(context, local, X64_GPR_RAX, block_index);

        x64_context_append(context,
                           x64_mov(x64_operand_gpr(X64_GPR_RAX),
                                   x64_operand_immediate(I.B_data.immediate)));
        x64_context_append(context, x64_idiv(x64_operand_alloc(C)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_IMMEDIATE: {
        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_Allocation *A = x64_context_allocate_to_gpr(
            context, local, X64_GPR_RAX, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_immediate(I.B_data.immediate)));

        x64_GPR gpr = x64_context_aquire_any_gpr(context, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_immediate(I.C_data.immediate)));

        x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_Allocation *A = x64_context_allocate_to_gpr(
            context, local, X64_GPR_RAX, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_immediate(I.B_data.immediate)));

        x64_GPR gpr = x64_context_aquire_any_gpr(context, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_constant(I.C_data.constant)));

        x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x64_codegen_divide_constant(Instruction I,
                                 u64 block_index,
                                 LocalVariable *local,
                                 x64_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_Allocation *C = x64_context_allocation_of(context, I.C_data.ssa);
        if ((C->location.kind == LOCATION_GPR) &&
            (C->location.gpr == X64_GPR_RAX)) {
            x64_context_reallocate_active(context, C);
        }

        x64_context_allocate_to_gpr(context, local, X64_GPR_RAX, block_index);

        x64_context_append(context,
                           x64_mov(x64_operand_gpr(X64_GPR_RAX),
                                   x64_operand_constant(I.B_data.constant)));
        x64_context_append(context, x64_idiv(x64_operand_alloc(C)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_IMMEDIATE: {
        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_Allocation *A = x64_context_allocate_to_gpr(
            context, local, X64_GPR_RAX, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_constant(I.B_data.constant)));

        x64_GPR gpr = x64_context_aquire_any_gpr(context, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_immediate(I.C_data.immediate)));

        x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_context_aquire_gpr(context, X64_GPR_RDX, block_index);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64_GPR_RDX), x64_operand_immediate(0)));

        x64_Allocation *A = x64_context_allocate_to_gpr(
            context, local, X64_GPR_RAX, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A),
                                   x64_operand_constant(I.B_data.constant)));

        x64_GPR gpr = x64_context_aquire_any_gpr(context, block_index);
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_constant(I.C_data.constant)));

        x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));

        x64_context_release_gpr(context, X64_GPR_RDX, block_index);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x64_codegen_divide(Instruction I,
                        u64 block_index,
                        x64_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = x64_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x64_codegen_divide_ssa(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_IMMEDIATE: {
        x64_codegen_divide_immediate(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_codegen_divide_constant(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
