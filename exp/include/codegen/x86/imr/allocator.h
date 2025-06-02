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
#include "codegen/x86/imr/register_pool.h"

typedef struct x86_RegisterAllocations {
    x86_RegisterPool pool;
    x86_Allocation **buffer;
} x86_RegisterAllocations;

typedef enum x86_StackAllocationKind {
    X86_STACK_ALLOCATION_KIND_ALLOCATION,
    X86_STACK_ALLOCATION_KIND_PADDING,
    X86_STACK_ALLOCATION_KIND_FREE,
} x86_StackAllocationKind;

typedef struct x86_StackAllocationData {
    i32 offset;
    union {
        x86_Allocation *allocation;
        u64             padding;
        u64             unused;
    };
} x86_StackAllocationData;

typedef struct x86_StackAllocation {
    x86_StackAllocationKind kind;
    x86_StackAllocationData data;
} x86_StackAllocation;

typedef struct x86_StackAllocations {
    i32                  size;
    u32                  count;
    u32                  capacity;
    x86_StackAllocation *buffer;
} x86_StackAllocations;

typedef struct x86_Allocations {
    u32              count;
    u32              capacity;
    x86_Allocation **buffer;
} x86_Allocations;

/**
 * @brief manages where SSA locals are allocated
 */
typedef struct x64_Allocator {
    x86_RegisterAllocations register_allocations;
    x86_StackAllocations    stack_allocations;
    x86_Allocations         allocations;
} x86_Allocator;

void x86_allocator_create(x86_Allocator *restrict allocator);
void x86_allocator_destroy(x86_Allocator *restrict allocator);

// bool x86_allocator_uses_stack(x86_Allocator *restrict allocator);
// i32  x86_allocator_total_stack_size(x86_Allocator *restrict allocator);

/*
 * #NOTE: Does it really make sense for the allocator to be adding instructions
 * to the function's body? Shouldn't the allocators concern simply be
 * bookkeeping of where the allocations are located? Like how the symbol table
 * isn't concerned with managing the memory of the constants it points to.
 */

/**
 * @brief Places the local into a location within the function's
 * frame of reference.
 */
x86_Allocation *x86_allocator_allocate_local(x86_Allocator *restrict allocator,
                                             Local const *restrict local,
                                             u32 block_index,
                                             Context *restrict context);

// /**
//  * @brief return the allocation for the given SSA
//  */
// x86_Allocation *x86_allocator_allocation_of(x86_Allocator *restrict
// allocator,
//                                             u32 ssa);

// /**
//  * @brief release the given GPR.
//  *
//  * @note This moves any existing living allocation to another GPR if able,
//  * otherwise it spills the allocation to the stack.
//  *
//  */
// void x86_allocator_release_gpr(x86_Allocator *restrict allocator,
//                                x86_GPR gpr,
//                                u64     Idx,
//                                struct x86_Function *restrict x86_function);

// /**
//  * @brief aquire the given GPR.
//  *
//  * @note This moves any existing living allocation to another GPR if able,
//  * otherwise it spills the allocation to the stack.
//  */
// void x86_allocator_aquire_gpr(x86_Allocator *restrict allocator,
//                               x86_GPR gpr,
//                               u64     Idx,
//                               struct x86_Function *restrict x86_function);

// /**
//  * @brief allocate the given SSA local.
//  *
//  * @note The allocation strategy is the "linear-scan" algorithm.
//  *       essentially, allocate to the next available GPR, or spill
//  *       the oldest active allocation to the stack.
//  */
// x86_Allocation *
// x86_allocator_allocate(x86_Allocator *restrict allocator,
//                        u64    Idx,
//                        Local *local,
//                        struct x86_Function *restrict x86_function);

// /**
//  * @brief allocate the given SSA local from the active allocation.
//  *
//  * @note This is used when we have an allocation in a GPR that we
//  * would like to reuse. If the allocation is dead after the current
//  * instruction, we can reuse it. Otherwise, we have to spill it.
//  */
// x86_Allocation *
// x86_allocator_allocate_from_active(x86_Allocator *restrict allocator,
//                                    u64             Idx,
//                                    Local          *local,
//                                    x86_Allocation *active,
//                                    struct x86_Function *restrict
//                                    x86_function);

// /**
//  * @brief allocate the given SSA local to the next available GPR
//  *
//  * @note This always allocates the SSA local to a GPR, thus it spills the
//  oldest
//  *       active allocation to the stack if needed. We use this when
//  *       we need the local in a GPR, but we don't care which one. i.e. for
//  *       immediate operands.
//  */
// x86_Allocation *
// x86_allocator_allocate_to_any_gpr(x86_Allocator *restrict allocator,
//                                   Local *local,
//                                   struct x86_Function *restrict
//                                   x86_function);

// /**
//  * @brief allocate the given SSA local to the given GPR.
//  *
//  * @note This forces the local to be allocated to the given GPR,
//  * spilling the current allocation if needed.
//  */
// x86_Allocation *
// x86_allocator_allocate_to_gpr(x86_Allocator *restrict allocator,
//                               Local  *local,
//                               x86_GPR gpr,
//                               u64     Idx,
//                               struct x86_Function *restrict x86_function);

// /**
//  * @brief allocate the given SSA local to the stack.
//  *
//  * @note This is used to "allocate" incoming arguments to the stack
//  * due to the calling convention. It trusts the caller to pass in
//  * the correct offset.
//  */
// x86_Allocation *x86_allocator_allocate_to_stack(
//     x86_Allocator *restrict allocator, i64 offset, Local *local);

// /**
//  * @brief allocate the result of a function.
//  *
//  * @note This is used to allocate the incoming result of a function.
//  * as the caller knows the location of the result.
//  */
// x86_Allocation *x86_allocator_allocate_result(x86_Allocator *restrict
// allocator,
//                                               x86_Location location,
//                                               Type const  *type);

// /**
//  * @brief reallocate the given active allocation.
//  *
//  * @note This is used when we need the given allocation to stick around,
//  * but we need to move it to a different GPR. i.e. when generating the idiv
//  * instruction which always uses rAX as an operand. If the allocation is
//  * on the stack, this is a no-op.
//  */
// void x86_allocator_reallocate_active(
//     x86_Allocator *restrict allocator,
//     x86_Allocation *restrict active,
//     struct x86_Function *restrict x86_function);

// /**
//  * @brief temporarily aquire the next available GPR.
//  *
//  * @note This is used when we need a GPR just for the current instruction,
//  but
//  * we don't care which one. i.e. for immediate operands. This spills the
//  oldest
//  * active allocation to the stack if needed. This is distinct from
//  * x86_allocator_allocate_to_any_gpr in that it doesn't mark the GPR as used.
//  * So you cannot rely on the value of the GPR after any other allocations.
//  */
// x86_GPR
// x86_allocator_aquire_any_gpr(x86_Allocator *restrict allocator,
//                              u64 size,
//                              u64 Idx,
//                              struct x86_Function *restrict x86_function);

#endif // !EXP_BACKEND_X86_ALLOCATOR_H
