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

#include "backend/x64/allocation.h"
#include "backend/x64/location.h"
#include "utility/alloc.h"

x64_Allocation *x64_allocation_allocate() {
    x64_Allocation *allocation = callocate(1, sizeof(x64_Allocation));
    return allocation;
}

void x64_allocation_deallocate(x64_Allocation *restrict allocation) {
    deallocate(allocation);
}

bool x64_allocation_alive(x64_Allocation *allocation, u64 block_index) {
    return allocation->lifetime.last_use >= block_index;
}

bool x64_allocation_location_equality(x64_Allocation *restrict allocation,
                                      x64_Location location) {
    return x64_location_equality(location, allocation->location);
}
