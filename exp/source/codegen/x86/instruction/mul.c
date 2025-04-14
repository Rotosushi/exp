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

#include "codegen/x86/instruction/mul.h"
#include "support/message.h"
#include "support/unreachable.h"

static void x86_codegen_multiply_ssa(Instruction    I,
                                     u64            block_index,
                                     LocalVariable *local,
                                     x86_Context   *context) {
    x86_Allocation *B = x86_context_allocation_of(context, I.B_data.ssa);
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_ssa: ssa"), stdout);
        }
        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        /*
         * #TODO: GPR location equality needs to take into account overlapping
         * registers. i.e. register a, ax, eax, rax, all refer to the "same"
         * register. so while the naieve equality will say eax and rax are
         * different, they are strictly speaking the same register.
         *
         * #TODO: When dealing with a GPR in arithmetic operations, we need to
         * allow for the different sizes of available registers, based on the
         * size of the incoming operands.
         */
        if (x86_location_eq(B->location, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_allocate_from_active(context, local, B, block_index);

            x86_context_release_gpr(context, X86_GPR_RDX, block_index);

            x86_context_append(context, x86_imul(x86_operand_alloc(C)));
            break;
        }

        if (x86_location_eq(C->location, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_allocate_from_active(context, local, C, block_index);

            x86_context_release_gpr(context, X86_GPR_RDX, block_index);

            x86_context_append(context, x86_imul(x86_operand_alloc(B)));
            break;
        }

        x86_context_allocate_to_gpr(context, local, X86_GPR_RAX, block_index);
        x86_context_release_gpr(context, X86_GPR_RDX, block_index);
        if ((B->lifetime.last_use <= C->lifetime.last_use)) {
            x86_context_append(
                context,
                x86_mov(x86_operand_gpr(X86_GPR_RAX), x86_operand_alloc(B)));
            x86_context_append(context, x86_imul(x86_operand_alloc(C)));
        } else {
            x86_context_append(
                context,
                x86_mov(x86_operand_gpr(X86_GPR_RAX), x86_operand_alloc(C)));
            x86_context_append(context, x86_imul(x86_operand_alloc(B)));
        }
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_ssa: i64"), stdout);
        }
        if (x86_allocation_location_eq(B, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_allocate_from_active(context, local, B, block_index);

            x86_context_release_gpr(context, X86_GPR_RDX, block_index);
            x86_context_append(context,
                               x86_mov(x86_operand_gpr(X86_GPR_RDX),
                                       x86_operand_immediate(I.C_data.i64_)));
            x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
            break;
        }

        x86_context_allocate_to_gpr(context, local, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_immediate(I.C_data.i64_)));
        x86_context_append(context, x86_imul(x86_operand_alloc(B)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_ssa: constant"), stdout);
        }
        if (x86_allocation_location_eq(B, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_allocate_from_active(context, local, B, block_index);

            x86_context_release_gpr(context, X86_GPR_RDX, block_index);
            x86_context_append(
                context,
                x86_mov(x86_operand_gpr(X86_GPR_RDX),
                        x86_operand_constant(I.C_data.constant)));
            x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
            break;
        }

        x86_context_allocate_to_gpr(context, local, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_constant(I.C_data.constant)));
        x86_context_append(context, x86_imul(x86_operand_alloc(B)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x86_codegen_multiply_immediate(Instruction    I,
                                           u64            block_index,
                                           LocalVariable *local,
                                           x86_Context   *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_immediate: ssa"), stdout);
        }
        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        if (x86_location_eq(C->location, x86_location_gpr(X86_GPR_RAX))) {
            x86_context_allocate_from_active(context, local, C, block_index);

            x86_context_release_gpr(context, X86_GPR_RDX, block_index);
            x86_context_append(context,
                               x86_mov(x86_operand_gpr(X86_GPR_RDX),
                                       x86_operand_immediate(I.B_data.i64_)));
            x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
            break;
        }

        x86_context_allocate_to_gpr(context, local, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_immediate(I.B_data.i64_)));
        x86_context_append(context, x86_imul(x86_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_immediate: i64"), stdout);
        }
        x86_Allocation *A = x86_context_allocate_to_gpr(
            context, local, X86_GPR_RAX, block_index);
        x86_context_release_gpr(context, X86_GPR_RDX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_immediate(I.B_data.i64_)));
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RDX),
                                   x86_operand_immediate(I.C_data.i64_)));
        x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_immediate: constant"), stdout);
        }
        x86_Allocation *A = x86_context_allocate_to_gpr(
            context, local, X86_GPR_RAX, block_index);
        x86_context_release_gpr(context, X86_GPR_RDX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_immediate(I.B_data.i64_)));
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RDX),
                                   x86_operand_constant(I.C_data.constant)));
        x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x86_codegen_multiply_constant(Instruction    I,
                                   u64            block_index,
                                   LocalVariable *local,
                                   x86_Context   *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_constant: ssa"), stdout);
        }
        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        if ((C->location.kind == X86_LOCATION_GPR) &&
            (C->location.gpr == X86_GPR_RAX)) {
            x86_context_allocate_from_active(context, local, C, block_index);

            x86_context_release_gpr(context, X86_GPR_RDX, block_index);
            x86_context_append(
                context,
                x86_mov(x86_operand_gpr(X86_GPR_RDX),
                        x86_operand_constant(I.B_data.constant)));
            x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
            break;
        }

        x86_context_allocate_to_gpr(context, local, X86_GPR_RAX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RAX),
                                   x86_operand_constant(I.B_data.constant)));
        x86_context_append(context, x86_imul(x86_operand_alloc(C)));
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_constant: i64"), stdout);
        }
        x86_Allocation *A = x86_context_allocate_to_gpr(
            context, local, X86_GPR_RAX, block_index);
        x86_context_release_gpr(context, X86_GPR_RDX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_constant(I.B_data.constant)));
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RDX),
                                   x86_operand_immediate(I.C_data.i64_)));
        x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_multiply_constant: constant"), stdout);
        }
        x86_Allocation *A = x86_context_allocate_to_gpr(
            context, local, X86_GPR_RAX, block_index);
        x86_context_release_gpr(context, X86_GPR_RDX, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_constant(I.B_data.constant)));
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(X86_GPR_RDX),
                                   x86_operand_constant(I.C_data.constant)));
        x86_context_append(context, x86_imul(x86_operand_gpr(X86_GPR_RDX)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x86_codegen_mul(Instruction I,
                     u64         block_index,
                     x86_Context *restrict context) {
    /*
    #NOTE:
      imul takes a single reg/mem argument,
      and expects the other argument to be in %rax
      and stores the result in %rdx:%rax.
    */
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = x86_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x86_codegen_multiply_ssa(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_I64: {
        x86_codegen_multiply_immediate(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_codegen_multiply_constant(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
