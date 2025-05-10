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
#include <stddef.h>

#include "codegen/x86/imr/locations.h"
#include "support/allocation.h"
#include "support/assert.h"

void x86_locations_create(x86_Locations *restrict locations) {
    exp_assert(locations != NULL);
    locations->capacity = 0;
    locations->buffer   = NULL;
}

void x86_locations_destroy(x86_Locations *restrict locations) {
    exp_assert(locations != NULL);
    deallocate(locations->buffer);
    x86_locations_create(locations);
}

void x86_locations_allocate(x86_Locations *restrict locations, u32 capacity) {
    exp_assert(locations != NULL);
    locations->capacity = capacity;
    locations->buffer   = callocate(capacity, sizeof(*locations->buffer));
}

x86_Location *x86_locations_at(x86_Locations const *restrict locations,
                               u32 ssa) {
    exp_assert(locations != NULL);
    exp_assert(locations->capacity > ssa);
    return locations->buffer + ssa;
}
