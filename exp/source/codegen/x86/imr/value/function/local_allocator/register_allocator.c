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

#include "codegen/x86/imr/local_allocator/register_allocator.h"
#include "support/allocation.h"

void x86_register_allocator_create(
    x86_RegisterAllocator *restrict register_allocations) {
    x86_register_pool_initialize(&register_allocations->pool);
    register_allocations->gpr_buffer =
        callocate(x86_gpr_count(), sizeof(*register_allocations->gpr_buffer));
}

void x86_register_allocator_destroy(
    x86_RegisterAllocator *restrict register_allocations) {
    x86_register_pool_initialize(&register_allocations->pool);
    deallocate(register_allocations->gpr_buffer);
}

void x86_register_allocator_aquire_gpr(
    x86_RegisterAllocator *restrict register_allocator, x86_GPR gpr) {
    EXP_ASSERT(register_allocator != NULL);
    x86_register_pool_gpr_aquire(&register_allocator->pool, gpr);
}

void x86_register_allocator_release_gpr(
    x86_RegisterAllocator *restrict register_allocator, x86_GPR gpr) {
    EXP_ASSERT(register_allocator != NULL);
    x86_register_pool_gpr_release(&register_allocator->pool, gpr);
}

/**
 * @brief allocate the given allocation to the next available GPR
 *
 * @pre the allocation must be a valid size for a GPR.
 *
 * @return true if able to allocate
 * @return false otherwise
 */
bool x86_register_allocator_allocate_to_next_available(
    x86_RegisterAllocator *restrict register_allocations,
    x86_Allocation *restrict allocation) {
    x86_RegisterPool *pool       = &register_allocations->pool;
    x86_Allocation  **gpr_buffer = register_allocations->gpr_buffer;
    u8                gpr_index;
    if (!x86_register_pool_gpr_aquire_next_available(pool, &gpr_index)) {
        return false;
    }

    u64 size = x86_allocation_size_of(allocation);
    EXP_ASSERT(x86_gpr_valid_size(size));
    x86_GPR gpr           = x86_gpr_with_size(gpr_index, size);
    allocation->location  = x86_location_gpr(gpr);
    gpr_buffer[gpr_index] = allocation;
    return true;
}

static void x86_register_allocator_gpr_release_expired(
    x86_RegisterAllocator *restrict register_allocator, u32 block_index) {
    x86_Allocation **gpr_buffer = register_allocator->gpr_buffer;
    // #NOTE: the buffer of allocations here is "sparse", in that we only
    // fill buffer[index] when index refers to a register which has been
    // allocated.
    for (u32 index = 0; index < x86_gpr_count(); ++index) {
        x86_Allocation *allocation = gpr_buffer[index];
        if (allocation == NULL) { continue; }
        if (x86_allocation_alive(allocation, block_index)) { continue; }

        x86_allocation_expire(allocation);
        gpr_buffer[index] = NULL;
    }
}

void x86_register_allocator_release_expired(
    x86_RegisterAllocator *restrict register_allocator, u32 block_index) {
    EXP_ASSERT(register_allocator != NULL);
    // #NOTE we have no way of verifying the block_index as valid at this
    // point, so it simply has to be a precondition.
    x86_register_allocator_gpr_release_expired(register_allocator, block_index);
}
