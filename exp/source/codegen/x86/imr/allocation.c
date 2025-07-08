/**
 * Copyright (C) 2024 Cade Weinberg
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
#include <stddef.h>

#include "codegen/x86/imr/allocation.h"
#include "codegen/x86/intrinsics/ptr_kind_of.h"
#include "support/allocation.h"
#include "support/assert.h"

x86_Allocation *x86_allocation_allocate(Local const *restrict local) {
    exp_assert(local != NULL);
    x86_Allocation *allocation = callocate(1, sizeof(x86_Allocation));
    allocation->ssa            = local->ssa;
    allocation->alive          = true;
    allocation->name           = local->name;
    allocation->lifetime       = local->lifetime;
    allocation->type           = local->type;
    return allocation;
}

void x86_allocation_deallocate(x86_Allocation *restrict allocation) {
    exp_assert(allocation != NULL);
    deallocate(allocation);
}

bool x86_allocation_alive(x86_Allocation const *restrict allocation,
                          u32 block_index) {
    exp_assert(allocation != NULL);
    if (!allocation->alive) { return false; }
    if (block_index < allocation->lifetime.start) { return false; }
    return block_index <= allocation->lifetime.end;
}

void x86_allocation_expire(x86_Allocation *restrict allocation) {
    exp_assert(allocation != NULL);
    exp_assert(allocation->alive);
    allocation->alive    = false;
    allocation->location = x86_location_expire();
}

u64 x86_allocation_size_of(x86_Allocation const *restrict allocation) {
    exp_assert(allocation != NULL);
    return layout_size_of(allocation->type->layout);
}

u64 x86_allocation_align_of(x86_Allocation const *restrict allocation) {
    exp_assert(allocation != NULL);
    return layout_align_of(allocation->type->layout);
}

x86_PtrKind
x86_allocation_ptr_kind_of(x86_Allocation const *restrict allocation) {
    exp_assert(allocation != NULL);
    return x86_ptr_kind_of(allocation->type->layout);
}
