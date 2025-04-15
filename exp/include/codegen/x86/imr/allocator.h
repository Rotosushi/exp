// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X86_ALLOCATOR_H
#define EXP_BACKEND_X86_ALLOCATOR_H

#include "codegen/x86/imr/allocation.h"
#include "codegen/x86/imr/bytecode.h"
#include "codegen/x86/imr/registers.h"

/**
 * @brief General Purpose Register Pool
 *
 */
typedef struct x86_GPRP {
    u16              bitset;
    x86_Allocation **buffer;
} x86_GPRP;

typedef struct x86_StackAllocations {
    i64              active_stack_size;
    i64              total_stack_size;
    u64              count;
    u64              capacity;
    x86_Allocation **buffer;
} x86_StackAllocations;

typedef struct x86_AllocationBuffer {
    u64              count;
    u64              capacity;
    x86_Allocation **buffer;
} x86_AllocationBuffer;

/**
 * @brief manages where SSA locals are allocated
 *
 */
typedef struct x64_Allocator {
    x86_GPRP             gprp;
    x86_StackAllocations stack_allocations;
    x86_AllocationBuffer allocations;
} x86_Allocator;

void x86_allocator_create(x86_Allocator *restrict allocator);
void x86_allocator_destroy(x86_Allocator *restrict allocator);

bool x86_allocator_uses_stack(x86_Allocator *restrict allocator);
i64  x86_allocator_total_stack_size(x86_Allocator *restrict allocator);

/**
 * @brief return the allocation for the given SSA
 */
x86_Allocation *x86_allocator_allocation_of(x86_Allocator *restrict allocator,
                                            u64 ssa);

/**
 * @brief release the given GPR.
 *
 * @note This moves any existing living allocation to another GPR if able,
 * otherwise it spills the allocation to the stack.
 *
 */
void x86_allocator_release_gpr(x86_Allocator *restrict allocator,
                               x86_GPR gpr,
                               u64     Idx,
                               x86_Bytecode *restrict x64bc);

/**
 * @brief aquire the given GPR.
 *
 * @note This moves any existing living allocation to another GPR if able,
 * otherwise it spills the allocation to the stack.
 */
void x86_allocator_aquire_gpr(x86_Allocator *restrict allocator,
                              x86_GPR gpr,
                              u64     Idx,
                              x86_Bytecode *restrict x64bc);

/**
 * @brief allocate the given SSA local.
 *
 * @note The allocation strategy is the "linear-scan" algorithm.
 *       essentially, allocate to the next available GPR, or spill
 *       the oldest active allocation to the stack.
 */
x86_Allocation *x86_allocator_allocate(x86_Allocator *restrict allocator,
                                       u64    Idx,
                                       Local *local,
                                       x86_Bytecode *restrict x64bc);

/**
 * @brief allocate the given SSA local from the active allocation.
 *
 * @note This is used when we have an allocation in a GPR that we
 * would like to reuse. If the allocation is dead after the current
 * instruction, we can reuse it. Otherwise, we have to spill it.
 */
x86_Allocation *
x86_allocator_allocate_from_active(x86_Allocator *restrict allocator,
                                   u64             Idx,
                                   Local          *local,
                                   x86_Allocation *active,
                                   x86_Bytecode *restrict x64bc);

/**
 * @brief allocate the given SSA local to the next available GPR
 *
 * @note This always allocates the SSA local to a GPR, thus it spills the oldest
 *       active allocation to the stack if needed. We use this when
 *       we need the local in a GPR, but we don't care which one. i.e. for
 *       immediate operands.
 */
x86_Allocation *
x86_allocator_allocate_to_any_gpr(x86_Allocator *restrict allocator,
                                  Local *local,
                                  x86_Bytecode *restrict x64bc);

/**
 * @brief allocate the given SSA local to the given GPR.
 *
 * @note This forces the local to be allocated to the given GPR,
 * spilling the current allocation if needed.
 */
x86_Allocation *x86_allocator_allocate_to_gpr(x86_Allocator *restrict allocator,
                                              Local  *local,
                                              x86_GPR gpr,
                                              u64     Idx,
                                              x86_Bytecode *restrict x64bc);

/**
 * @brief allocate the given SSA local to the stack.
 *
 * @note This is used to "allocate" incoming arguments to the stack
 * due to the calling convention. It trusts the caller to pass in
 * the correct offset.
 */
x86_Allocation *x86_allocator_allocate_to_stack(
    x86_Allocator *restrict allocator, i64 offset, Local *local);

/**
 * @brief allocate the result of a function.
 *
 * @note This is used to allocate the incoming result of a function.
 * as the caller knows the location of the result.
 */
x86_Allocation *x86_allocator_allocate_result(x86_Allocator *restrict allocator,
                                              x86_Location location,
                                              Type const  *type);

/**
 * @brief reallocate the given active allocation.
 *
 * @note This is used when we need the given allocation to stick around,
 * but we need to move it to a different GPR. i.e. when generating the idiv
 * instruction which always uses rAX as an operand. If the allocation is
 * on the stack, this is a no-op.
 */
void x86_allocator_reallocate_active(x86_Allocator *restrict allocator,
                                     x86_Allocation *restrict active,
                                     x86_Bytecode *restrict x64bc);

/**
 * @brief aquire the next available GPR.
 *
 * @note This is used when we need a temporary GPR, but we don't care which one.
 *       i.e. for immediate operands. This spills the oldest active
 *       allocation to the stack if needed. This is distinct from
 *       x86_allocator_allocate_to_any_gpr in that it doesn't
 *       mark the GPR as used.
 */
x86_GPR x86_allocator_aquire_any_gpr(x86_Allocator *restrict allocator,
                                     u64 size,
                                     u64 Idx,
                                     x86_Bytecode *restrict x64bc);

#endif // !EXP_BACKEND_X86_ALLOCATOR_H
