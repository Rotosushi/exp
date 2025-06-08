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

#include "codegen/x86/imr/incoming_argument_allocator.h"
#include "codegen/x86/imr/allocation.h"
#include "codegen/x86/imr/registers.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void x86_incoming_argument_allocator_create(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator) {
    exp_assert(incoming_argument_allocator != NULL);
    incoming_argument_allocator->registers_used = 0;
    incoming_argument_allocator->length         = 0;
    incoming_argument_allocator->capacity       = 0;
    incoming_argument_allocator->buffer         = NULL;
}

void x86_incoming_argument_allocator_destroy(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator) {
    exp_assert(incoming_argument_allocator != NULL);
    deallocate(incoming_argument_allocator->buffer);
    x86_incoming_argument_allocator_create(incoming_argument_allocator);
}

static bool x86_incoming_argument_allocator_full(
    x86_IncomingArgumentAllocator const *restrict incoming_argument_allocator) {
    return (incoming_argument_allocator->length + 1) >=
           incoming_argument_allocator->capacity;
}

static void x86_incoming_argument_allocator_grow(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator) {
    Growth_u8 g = array_growth_u8(incoming_argument_allocator->capacity,
                                  sizeof(*incoming_argument_allocator->buffer));
    incoming_argument_allocator->buffer =
        reallocate(incoming_argument_allocator->buffer, g.alloc_size);
    incoming_argument_allocator->capacity = g.new_capacity;
}

static bool x86_incoming_argument_allocator_register_allocate(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator,
    x86_Allocation *restrict allocation) {
    if (incoming_argument_allocator->registers_used >= 6) { return false; }
    u64 size = x86_allocation_size_of(allocation);
    if (!x86_gpr_valid_size(size)) { return false; }
    // #NOTE: We allocate the first six arguments which fit into general purpose
    // registers into those registers, starting from the left of the argument
    // list and working our way right.
    x86_GPR gpr =
        x86_gpr_for_argument(incoming_argument_allocator->registers_used, size);
}

void x86_incoming_argument_allocator_allocate(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator,
    x86_Allocation *restrict allocation) {
    exp_assert(incoming_argument_allocator != NULL);
    exp_assert(allocation != NULL);
}
