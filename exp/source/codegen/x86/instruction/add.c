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

#include "codegen/x86/instruction/add.h"
#include "support/message.h"
#include "support/unreachable.h"

// #TODO: I think I discovered a bug in the codegen for all arithemtic
// operations. We need to make sure that the result of the operation is
// never the memory location of a local variable. Since that would modify
// the value of the local variable.

static void x86_codegen_add_ssa(Instruction  I,
                                u64          block_index,
                                Local       *local,
                                x86_Context *context) {
    x86_Allocation *B = x86_context_allocation_of(context, I.B_data.ssa);
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_ssa: ssa"), stdout);
        }
        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        // if B or C is in a gpr we use it as the allocation point of A
        // and the destination operand of the x64 add instruction.
        // this is to try and keep the result, A, in a register.
        // We must be careful to not use a memory operand as the destination
        // of the x64 add instruction since that would modify the value
        // of the local variable. And since function arguments are also
        // constant, and these may be passed in registers, we can overwrite
        // their value if we are not careful.
        if (B->location.kind == X86_LOCATION_GPR) {
            x86_Allocation *A = x86_context_allocate_from_active(
                context, local, B, block_index);
            x86_context_append(
                context, x86_add(x86_operand_alloc(A), x86_operand_alloc(C)));
            return;
        }

        if (C->location.kind == X86_LOCATION_GPR) {
            x86_Allocation *A = x86_context_allocate_from_active(
                context, local, C, block_index);
            x86_context_append(
                context, x86_add(x86_operand_alloc(A), x86_operand_alloc(B)));
            return;
        }

        // since B and C are memory operands we have to move B or C
        // to a reg and then add.
        x86_Allocation *A = x86_context_allocate_to_any_gpr(context, local);

        // we use the huristic of longest lifetime to choose
        // which of B and C to move into A's gpr.
        if (B->lifetime.end <= C->lifetime.end) {
            x86_context_append(
                context, x86_mov(x86_operand_alloc(A), x86_operand_alloc(C)));
            x86_context_append(
                context, x86_add(x86_operand_alloc(A), x86_operand_alloc(B)));
        } else {
            x86_context_append(
                context, x86_mov(x86_operand_alloc(A), x86_operand_alloc(B)));
            x86_context_append(
                context, x86_add(x86_operand_alloc(A), x86_operand_alloc(C)));
        }
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_ssa: i64"), stdout);
        }
        x86_Allocation *A =
            x86_context_allocate_from_active(context, local, B, block_index);

        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_immediate(I.C_data.i64_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_ssa: constant"), stdout);
        }
        x86_Allocation *A =
            x86_context_allocate_from_active(context, local, B, block_index);

        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_constant(I.C_data.constant)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x86_codegen_add_immediate(Instruction  I,
                                      u64          block_index,
                                      Local       *local,
                                      x86_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_immediate: ssa"), stdout);
        }
        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        x86_Allocation *A =
            x86_context_allocate_from_active(context, local, C, block_index);

        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_immediate(I.B_data.i64_)));
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_immediate: i64"), stdout);
        }
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_immediate(I.B_data.i64_)));
        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_immediate(I.C_data.i64_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_immediate: constant"), stdout);
        }
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_constant(I.B_data.constant)));

        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_immediate(I.C_data.i64_)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

static void x86_codegen_add_constant(Instruction  I,
                                     u64          block_index,
                                     Local       *local,
                                     x86_Context *context) {
    switch (I.C_kind) {
    case OPERAND_KIND_SSA: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_constant: ssa"), stdout);
        }
        x86_Allocation *C = x86_context_allocation_of(context, I.C_data.ssa);
        x86_Allocation *A =
            x86_context_allocate_from_active(context, local, C, block_index);

        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_constant(I.B_data.constant)));
        break;
    }

    case OPERAND_KIND_I64: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_constant: i64"), stdout);
        }
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_constant(I.B_data.constant)));

        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_immediate(I.C_data.i64_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        if (context_trace(context->context)) {
            trace(SV("x64_codegen_add_constant: constant"), stdout);
        }
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        x86_context_append(context,
                           x86_mov(x86_operand_alloc(A),
                                   x86_operand_constant(I.B_data.constant)));

        x86_context_append(context,
                           x86_add(x86_operand_alloc(A),
                                   x86_operand_constant(I.C_data.constant)));
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x86_codegen_add(Instruction I,
                     u64         block_index,
                     x86_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = x86_context_lookup_ssa(context, I.A_data.ssa);
    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x86_codegen_add_ssa(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_I64: {
        x86_codegen_add_immediate(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_codegen_add_constant(I, block_index, local, context);
        break;
    }

    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}
