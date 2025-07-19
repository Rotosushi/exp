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
#ifndef EXP_CODEGEN_X86_IMR_LOCAL_ALLOCATOR_H
#define EXP_CODEGEN_X86_IMR_LOCAL_ALLOCATOR_H

#include "codegen/x86/imr/local_allocator/allocations.h"
#include "codegen/x86/imr/local_allocator/incoming_argument_allocator.h"
#include "codegen/x86/imr/local_allocator/register_allocator.h"
#include "codegen/x86/imr/local_allocator/stack_allocator.h"
#include "imr/value/function.h"

/**
 * @brief manages where SSA locals are allocated
 */
typedef struct x64_LocalAllocator {
    x86_RegisterAllocator         register_allocator;
    x86_StackAllocator            stack_allocator;
    x86_IncomingArgumentAllocator incoming_argument_allocator;
    x86_Allocations               allocations;
} x86_LocalAllocator;

void x86_local_allocator_create(x86_LocalAllocator *restrict local_allocator);
void x86_local_allocator_destroy(x86_LocalAllocator *restrict local_allocator);

i32 x86_local_allocator_stack_size(
    x86_LocalAllocator *restrict local_allocator);

x86_Allocation *
x86_local_allocator_allocation_at(x86_LocalAllocator *restrict local_allocator,
                                  u32 ssa);

/*
 * #NOTE: Does it really make sense for the allocator to be adding instructions
 * to the function's body? Shouldn't the allocators concern simply be
 * bookkeeping of where the allocations are located? Like how the symbol table
 * isn't concerned with managing the memory of the constants it points to.
 *
 * Yes, That is the major design goal of the refactor of the allocator.
 * The situations where we need to insert instructions alongside calls
 * to the allocator, for things like relocating an allocation that is
 * in the way of the correct operation of an instruction, say an unrellated
 * local is located in rax and we are trying to divide, we are going to
 * have that functionality placed into an "intrinsic" function.
 * And what I mean by "intrinsic" here is that these functions are the
 * building blocks of code generation. Not necessarily that we are going to
 * expose all of them to user code a'la "compiler intrinsics." This
 * misnomer is a holdover from when size_of and align_of were implemented
 * here. And so a renaming might be prudent, but it's not happend yet,
 * and I don't have a good name. "utility", "support", "common",
 * "primitives", "atom". actually I like "atom." It brings up the idea
 * of basic building block used in combination to create larger structure.
 * "intrinsics" like size_of are also used as atoms within code generation.
 * so maybe it's an "is-a" relationship. :)
 * intrinsics "are" atoms, but atoms "are not necessarily" intrinsics.
 *
 */

void x86_local_allocator_aquire_gpr(x86_LocalAllocator *restrict allocator,
                                    x86_GPR gpr);

void x86_local_allocator_release_gpr(x86_LocalAllocator *restrict allocator,
                                     x86_GPR gpr);

/**
 * @brief Release all local allocations whose lifetime bounds do not
 * include the given block_index
 */
void x86_local_allocator_release_expired(x86_LocalAllocator *restrict allocator,
                                         u32 block_index);

/**
 * @brief Allocates the given local into a location within the function's
 * frame of reference. Can be either a register or the stack.
 */
x86_Allocation *
x86_local_allocator_allocate_local(x86_LocalAllocator *restrict local_allocator,
                                   Local const *restrict local,
                                   u32 block_index);

struct x86_FormalArgumentList;
x86_Allocation *x86_local_allocator_allocate_result(
    x86_LocalAllocator *restrict local_allocator,
    Local const *restrict local,
    struct x86_FormalArgumentList *restrict x86_arguments);

void x86_local_allocator_allocate_incoming_arguments(
    x86_LocalAllocator *restrict local_allocator,
    FormalArgumentList const *restrict arguments,
    struct x86_FormalArgumentList *restrict x86_arguments);

#endif // !EXP_CODEGEN_X86_IMR_LOCAL_ALLOCATOR_H
