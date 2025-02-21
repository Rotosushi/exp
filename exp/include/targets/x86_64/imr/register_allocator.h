// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file targets/x86_64/imr/register_allocator.h
 */

#ifndef EXP_TARGETS_X86_64_REGISTER_ALLOCATOR_H
#define EXP_TARGETS_X86_64_REGISTER_ALLOCATOR_H

#include "targets/x86_64/imr/allocation.h"
#include "utility/bitset.h"

typedef struct x86_64_Allocations {
    u32 length;
    u32 capacity;
    x86_64_Allocation *buffer;
} x86_64_Allocations;

typedef struct x86_64_RegisterAllocator {
    Bitset active_general_purpose_registers;
    i32 active_stack_size;
    i32 total_stack_size;
    x86_64_Allocations allocations;
} x86_64_RegisterAllocator;

struct Function;
struct Context;
void x86_64_register_allocator_initialize(
    x86_64_RegisterAllocator *register_allocator, struct Function *function,
    struct Context *context);
void x86_64_register_allocator_terminate(
    x86_64_RegisterAllocator *register_allocator);

/**
 * @brief allocates space in the current function's frame for
 * the given local. either in a register or on the stack.
 */
x86_64_Allocation *
x86_64_register_allocator_allocate(x86_64_RegisterAllocator *register_allocator,
                                   Local *local, u32 block_index);

/**
 * @brief moves an existing allocation to another location.
 */
void x86_64_register_allocator_reallocate_active(
    x86_64_RegisterAllocator *register_allocator, x86_64_Allocation *active,
    u32 block_index);
/**
 * @brief handles the case where we want to allocate space for
 *  a given local (ssa or label) using the contents of an existing
 *  allocation of a given local. allowing us the choice of reusing
 *  the existing space, or allocating a new location, and copying
 *  the contents.
 */
x86_64_Allocation *x86_64_regsiter_allocator_allocate_from_active(
    x86_64_RegisterAllocator *register_allocator, Local *local,
    x86_64_Allocation *active, u32 block_index);

/**
 * @brief handles the case where we need to allocate a given local to
 *  a specific GPR, such as the result allocation of a division or modulus
 *  operation.
 */
x86_64_Allocation *x86_64_register_allocator_allocate_to_gpr(
    x86_64_RegisterAllocator *register_allocator, Local *local, x86_64_GPR gpr,
    u32 block_index);

/**
 * @brief handles allocating outgoing arguments to the current stack frame
 *  until a better solution is found.
 *
 * @warning doesn't do any validation of the given address.
 */
x86_64_Allocation *x86_64_register_allocator_allocate_to_address(
    x86_64_RegisterAllocator *register_allocator, Local *local,
    x86_64_Address offset);

/**
 * @brief creates the allocation for the result of a function.
 * this is used from the callee's perspective, just denoting
 * where the result is to be placed.
 *
 * @note the call instruction allocates space for the
 * result of a function from the caller's perspective.
 */
x86_64_Allocation *
x86_64_allocator_allocate_result(x86_64_RegisterAllocator *register_allocator,
                                 x86_64_Location location, Type const *type);

#endif // EXP_TARGETS_X86_64_REGISTER_ALLOCATOR_H
