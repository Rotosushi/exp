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

#include "codegen/x86/instruction/mod.h"
#include "support/message.h"
#include "support/unreachable.h"

static void x86_codegen_modulus_ssa(Instruction  I,
                                    u64          block_index,
                                    Local       *local,
                                    x86_Context *context) {
    x86_Allocation *B = x86_context_allocation_of(context, I.B_data.ssa);
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        if (x86_location_eq(B->location, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_allocate_to_gpr(
                context, local, X86_GPR_RDX, block_index);
            x86_context_append(context,
                               x86_mov(x86_operand_gpr(X86_GPR_RDX),
                                       x86_operand_immediate(0)));

            x86_context_append(context, x86_idiv(x86_operand_alloc(C)));
            break;
        }

        if (x86_location_eq(C->location, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_allocate_to_gpr(
                context, local, X86_GPR_RDX, block_index);

            x86_context_reallocate_active(context, C);

            x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
            x86_context_append(
                context,
                x86_mov(x86_operand_gpr(X86_GPR_RAX), x86_operand_alloc(B)));

            x86_context_append(context, x86_idiv(x86_operand_alloc(C)));
            break;
        }

        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RAX), x86_operand_alloc(B)));

        x86_context_append(context, x86_idiv(x86_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RAX), x86_operand_alloc(B)));

        x86_GPR gpr = x86_context_aquire_any_gpr(context, 8, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(gpr),
                                   x86_operand_immediate(I.C_data.i64_)));

        x86_context_append(context, x86_idiv(x86_operand_gpr(gpr)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RAX), x86_operand_alloc(B)));

        x86_GPR gpr = x86_context_aquire_any_gpr(context, 8, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(gpr),
                                   x86_operand_constant(I.C_data.constant)));

        x86_context_append(context, x86_idiv(x86_operand_gpr(gpr)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x86_codegen_modulus_immediate(Instruction  I,
                                          u64          block_index,
                                          Local       *local,
                                          x86_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RDX), x86_operand_immediate(0)));

        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        if (x86_location_eq(C->location, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_reallocate_active(context, C);
        }

        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_immediate(I.B_data.i64_)));
        x86_context_append(context, x86_idiv(x86_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RDX), x86_operand_immediate(0)));

        x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_immediate(I.B_data.i64_)));

        x86_GPR gpr = x86_context_aquire_any_gpr(context, 8, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(gpr),
                                   x86_operand_immediate(I.C_data.i64_)));

        x86_context_append(context, x86_idiv(x86_operand_gpr(gpr)));
        x86_context_release_gpr(context, gpr, block_index);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RDX), x86_operand_immediate(0)));

        x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_immediate(I.B_data.i64_)));

        x86_GPR gpr = x86_context_aquire_any_gpr(context, 8, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(gpr),
                                   x86_operand_constant(I.C_data.constant)));

        x86_context_append(context, x86_idiv(x86_operand_gpr(gpr)));
        x86_context_release_gpr(context, gpr, block_index);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x86_codegen_modulus_constant(Instruction  I,
                                  u64          block_index,
                                  Local       *local,
                                  x86_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RDX), x86_operand_immediate(0)));

        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        if (x86_location_eq(C->location, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_reallocate_active(context, C);
        }

        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_constant(I.B_data.constant)));
        x86_context_append(context, x86_idiv(x86_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RDX), x86_operand_immediate(0)));

        x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_immediate(I.B_data.i64_)));

        x86_GPR gpr = x86_context_aquire_any_gpr(context, 8, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(gpr),
                                   x86_operand_constant(I.C_data.constant)));

        x86_context_append(context, x86_idiv(x86_operand_gpr(gpr)));
        x86_context_release_gpr(context, gpr, block_index);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_context_allocate_to_gpr(context, local, X86_GPR_RDX, block_index);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(X86_GPR_RDX), x86_operand_immediate(0)));

        x86_context_aquire_gpr(context, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_constant(I.B_data.constant)));

        x86_GPR gpr = x86_context_aquire_any_gpr(context, 8, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(gpr),
                                   x86_operand_constant(I.C_data.constant)));

        x86_context_append(context, x86_idiv(x86_operand_gpr(gpr)));
        x86_context_release_gpr(context, gpr, block_index);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x86_codegen_mod(Instruction I,
                     u64         block_index,
                     x86_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = x86_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x86_codegen_modulus_ssa(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_I64: {
        x86_codegen_modulus_immediate(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_codegen_modulus_constant(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
