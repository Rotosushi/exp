/**
 * Copyright (C) 2025 Cade Weinberg
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

#include "codegen/x86/imr/local_allocator/incoming_argument_allocator.h"
#include "support/assert.h"

void x86_incoming_argument_allocator_create(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator) {
    exp_assert(incoming_argument_allocator != NULL);
    incoming_argument_allocator->registers_used = 0;
    x86_stack_allocator_create(&incoming_argument_allocator->stack_arguments);
    // #HACK: Since we know that the stack slots for arguments are above the
    // stack frame relative to RBP, and the function call pushed the return
    // address onto the stack, and the function's header pushed the previous RBP
    // onto the stack. and the size of both of those pushes are 8, and the
    // stack_allocator assigns offsets based on it's current size. We want to
    // start allocating arguments with an initial offset of 16.
    incoming_argument_allocator->stack_arguments.size = 16;
}

void x86_incoming_argument_allocator_destroy(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator) {
    exp_assert(incoming_argument_allocator != NULL);
    incoming_argument_allocator->registers_used = 0;
    x86_stack_allocator_destroy(&incoming_argument_allocator->stack_arguments);
}

bool x86_incoming_argument_allocator_allocate_register(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator,
    x86_Allocation *restrict allocation) {
    exp_assert(incoming_argument_allocator != NULL);
    exp_assert(allocation != NULL);
    if (incoming_argument_allocator->registers_used >= 6) { return false; }

    u64 size = x86_allocation_size_of(allocation);
    if (!x86_gpr_valid_size(size)) { return false; }

    x86_GPR gpr = x86_gpr_for_argument(
        incoming_argument_allocator->registers_used++, size);
    allocation->location = x86_location_gpr(gpr);
    return true;
}

void x86_incoming_argument_allocator_allocate_stack(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator,
    x86_Allocation *restrict allocation) {
    exp_assert(incoming_argument_allocator != NULL);
    exp_assert(allocation != NULL);

    // #NOTE: While the stack_allocator accounts for removing allocations
    // in the middle of a functions runtime, that will never happen, However
    // we may end up with an argument placed into the padding between two
    // other arguments. Which I do not know if that is ABI conforming.
    // My assumption is no.
    x86_stack_allocator_allocate_to_next_available(
        &incoming_argument_allocator->stack_arguments, allocation);
    // #HACK: since we know that the incoming arguments are above the current
    // frame relative to RBP, and the stack allocator normally assumes downward
    // growth, we reverse the direction here to address above the current frame.
    allocation->location.offset = -allocation->location.offset;
}
