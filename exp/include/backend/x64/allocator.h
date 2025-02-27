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
#include "backend/x64/registers.h"
#include "utility/bitset.h"

// #TODO:
//  typedef struct {
//    Bitset bitset;
//  } x64_GPRegisterPool;
//  typedef struct {
//      i32 active_size;
//      i32 total_size;
//  } x64_StackFrame;
//  typedef struct {
//      u32 count;
//      u32 capacity;
//      x64_Allocation **buffer;
//  } x64_Allocations;
//  typedef struct {
//      x64_GPRegisterPool register_pool;
//      x64_StackFrame stack_frame;
//      x64_Allocations allocations;
//      Lifetimes lifetimes;
//      struct x64_Context *x64_context;
//  } x64_LocalRegisterAllocator;

/**
 * @brief General Purpose Register Pool
 *
 */
typedef struct x64_GPRP {
    Bitset bitset;
} x64_GPRegisterPool;

typedef struct x64_StackFrame {
    i32 active_size;
    i32 total_size;
} x64_StackFrame;

typedef struct x64_Allocations {
    u64 capacity;
    x64_Allocation **buffer;
} x64_Allocations;

/**
 * @brief manages where SSA locals are allocated
 */
typedef struct x64_LocalRegisterAllocator {
    x64_GPRegisterPool gprp;
    x64_StackFrame stack_frame;
    x64_Allocations allocations;
    Lifetimes lifetimes;
    struct x64_Context *context;
} x64_LocalRegisterAllocator;

void x64_allocator_initialize(x64_LocalRegisterAllocator *allocator,
                              FunctionBody *body,
                              struct x64_Context *x64_context);
void x64_allocator_terminate(x64_LocalRegisterAllocator *allocator);

bool x64_allocator_uses_stack(x64_LocalRegisterAllocator *allocator);
i32 x64_allocator_total_stack_size(x64_LocalRegisterAllocator *allocator);

void x64_allocator_update_lifetime(x64_LocalRegisterAllocator *allocator,
                                   u32 ssa,
                                   Lifetime lifetime);

x64_Allocation *
x64_allocator_allocation_at(x64_LocalRegisterAllocator *allocator, u64 ssa);

// this doesn't take into account when we need to keep the new allocation
// around and we are loading many gprs and the value gets overwritten
// by another new allocation. it's too fragile of an API. its something
// more suited for internal allocator use at best
void x64_allocator_release_gpr(x64_LocalRegisterAllocator *allocator,
                               x64_GPR gpr,
                               u64 block_index);
x64_GPR x64_allocator_aquire_any_gpr(x64_LocalRegisterAllocator *allocator,
                                     u64 block_index,
                                     u64 size);
void x64_allocator_aquire_gpr(x64_LocalRegisterAllocator *allocator,
                              x64_GPR gpr,
                              u64 block_index);
/**
 * @brief allocates space in the current function's frame for
 * the given local
 */
x64_Allocation *x64_allocator_allocate(x64_LocalRegisterAllocator *allocator,
                                       LocalVariable *local,
                                       u64 block_index);

void x64_allocator_reallocate_active(x64_LocalRegisterAllocator *allocator,
                                     x64_Allocation *active,
                                     u64 block_index);
/**
 * @brief handles the case where we want to allocate space for
 *  a given local (ssa or label) using the contents of an existing
 *  allocation of a given local.
 */
x64_Allocation *
x64_allocator_allocate_from_active(x64_LocalRegisterAllocator *allocator,
                                   u64 block_index,
                                   LocalVariable *local,
                                   x64_Allocation *active);

/**
 * @brief handles the case where we need to allocate a given local to
 *  a specific GPR, such as the result allocation of a division or modulus
 *  operation.
 */
x64_Allocation *
x64_allocator_allocate_to_gpr(x64_LocalRegisterAllocator *allocator,
                              LocalVariable *local,
                              x64_GPR gpr,
                              u64 block_index);

/**
 * @brief handles allocating outgoing arguments to the current stack frame
 *  untile a better solution is found.
 *
 * @warning doesn't do any validation of the given address.
 */
x64_Allocation *
x64_allocator_allocate_to_address(x64_LocalRegisterAllocator *allocator,
                                  LocalVariable *local,
                                  x64_Address offset);

/**
 * @brief creates the allocation for the result of a function.
 * this is used from the callee's perspective, just denoting
 * where the result is to be placed.
 *
 * @note the call instruction allocates space for the
 * result of a function from the caller's perspective.
 */
x64_Allocation *
x64_allocator_allocate_result(x64_LocalRegisterAllocator *allocator,
                              x64_Location location,
                              Type const *type);

#endif // !EXP_BACKEND_X64_ALLOCATOR_H
