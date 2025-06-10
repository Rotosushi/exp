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

#include "codegen/x86/imr/detail/allocations.h"
#include "support/allocation.h"
#include "support/array_growth.h"

void x86_allocations_create(x86_Allocations *restrict allocations) {
    allocations->length   = 0;
    allocations->capacity = 0;
    allocations->buffer   = NULL;
}

void x86_allocations_destroy(x86_Allocations *restrict allocations) {
    for (u32 index = 0; index < allocations->length; ++index) {
        deallocate(allocations->buffer[index]);
    }
    deallocate(allocations->buffer);
    x86_allocations_create(allocations);
}

static bool x86_allocations_full(x86_Allocations const *allocations) {
    return (allocations->length + 1) >= allocations->capacity;
}

static void x86_allocations_grow(x86_Allocations *restrict allocations) {
    Growth_u32 g =
        array_growth_u32(allocations->capacity, sizeof(*allocations->buffer));
    allocations->buffer   = reallocate(allocations->buffer, g.alloc_size);
    allocations->capacity = g.new_capacity;
}

x86_Allocation *x86_allocations_append(x86_Allocations *restrict allocations,
                                       Local const *restrict local,
                                       Context *restrict context) {
    if (x86_allocations_full(allocations)) {
        x86_allocations_grow(allocations);
    }

    x86_Allocation *allocation = x86_allocation_allocate(local, context);
    allocations->buffer[allocations->length++] = allocation;
    return allocation;
}
