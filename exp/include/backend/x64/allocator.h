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
#ifndef EXP_BACKEND_X64_ALLOCATOR_H
#define EXP_BACKEND_X64_ALLOCATOR_H

#include "backend/x64/allocation.h"
#include "backend/x64/bytecode.h"
#include "backend/x64/registers.h"
#include "utility/bitset.h"

/**
 * @brief General Purpose Register Pool
 *
 */
typedef struct x64_GPRP {
    Bitset bitset;
    x64_Allocation **buffer;
} x64_GPRP;

typedef struct x64_StackAllocations {
    i64 active_stack_size;
    i64 total_stack_size;
    u64 count;
    u64 capacity;
    x64_Allocation **buffer;
} x64_StackAllocations;

typedef struct x64_AllocationBuffer {
    u64 count;
    u64 capacity;
    x64_Allocation **buffer;
} x64_AllocationBuffer;

/**
 * @brief manages where SSA locals are allocated
 */
typedef struct x64_Allocator {
    x64_GPRP gprp;
    x64_StackAllocations stack_allocations;
    x64_AllocationBuffer allocations;
    Lifetimes lifetimes;
    struct x64_Context *context;
} x64_Allocator;

void x64_allocator_initialize(x64_Allocator *allocator,
                              FunctionBody *body,
                              struct x64_Context *x64_context);
void x64_allocator_terminate(x64_Allocator *allocator);

bool x64_allocator_uses_stack(x64_Allocator *allocator);
i64 x64_allocator_total_stack_size(x64_Allocator *allocator);

void x64_allocator_update_lifetime(x64_Allocator *allocator,
                                   u64 ssa,
                                   Lifetime lifetime);

x64_Allocation *x64_allocator_allocation_of(x64_Allocator *allocator, u64 ssa);

// this doesn't take into account when we need to keep the new allocation
// around and we are loading many gprs and the value gets overwritten
// by another new allocation. it's too fragile of an API. its something
// more suited for internal allocator use at best
void x64_allocator_release_gpr(x64_Allocator *allocator, x64_GPR gpr, u64 Idx);
x64_GPR x64_allocator_aquire_any_gpr(x64_Allocator *allocator, u64 Idx);
void x64_allocator_aquire_gpr(x64_Allocator *allocator, x64_GPR gpr, u64 Idx);
/**
 * @brief allocates space in the current function's frame for
 * the given local
 */
x64_Allocation *
x64_allocator_allocate(x64_Allocator *allocator, u64 Idx, LocalVariable *local);

/**
 * @brief handles the case where we want to allocate space for
 *  a given local (ssa or label) using the contents of an existing
 *  allocation of a given local.
 */
x64_Allocation *x64_allocator_allocate_from_active(x64_Allocator *allocator,
                                                   u64 Idx,
                                                   LocalVariable *local,
                                                   x64_Allocation *active);

/**
 * @brief handles the case where we need to allocate a given local to
 *  a specific GPR, such as the result allocation of a division or modulus
 *  operation.
 */
x64_Allocation *x64_allocator_allocate_to_gpr(x64_Allocator *allocator,
                                              x64_GPR gpr,
                                              u64 Idx,
                                              LocalVariable *local);

/**
 * @brief handles allocating outgoing arguments to the current stack frame
 *  temporarily.
 *
 * @warning doesn't do any validation of the given offset.
 * just directly creates an address relative to %rsp
 */
x64_Allocation *x64_allocator_allocate_to_stack(x64_Allocator *allocator,
                                                i64 offset,
                                                LocalVariable *local);

/**
 * @brief allocates space for the result of a function.
 * from the callee's perspective.
 *
 * @note the call instruction allocates space for the
 * result of a function from the caller's perspective.
 */
x64_Allocation *x64_allocator_allocate_result(x64_Allocator *allocator,
                                              x64_Location location,
                                              Type const *type);

void x64_allocator_reallocate_active(x64_Allocator *allocator,
                                     x64_Allocation *active);

// these smell like they are too low level of an API to be exposing,
// so I am double checking why they are used exactly.
// void x64_allocator_reallocate_active(x64_Allocator *allocator,
//                                     x64_Allocation *active);

//[[deprecated]] x64_GPR
// x64_allocator_spill_oldest_active(x64_Allocator *allocator);

#endif // !EXP_BACKEND_X64_ALLOCATOR_H
