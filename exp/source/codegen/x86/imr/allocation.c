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
#include "codegen/x86/imr/location.h"
#include "support/allocation.h"

x86_Allocation *x86_allocation_allocate() {
    x86_Allocation *allocation = callocate(1, sizeof(x86_Allocation));
    return allocation;
}

void x86_allocation_deallocate(x86_Allocation *restrict allocation) {
    deallocate(allocation);
}

bool x86_allocation_location_eq(x86_Allocation *restrict allocation,
                                x86_Location location) {
    return x86_location_equality(location, allocation->location);
}
