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

/**
 * @file imr/location.c
 */

#include "imr/location.h"

static Location location_construct(LocationKind kind, LocationData data) {
    Location location = {.kind = kind, .data = data};
    return location;
}

Location location_uninitialized() {
    return location_construct(LOCATION_UNINITIALIZED, (LocationData){});
}

Location location_register(u32 scope, u8 register_) {
    return location_construct(
        LOCATION_REGISTER,
        (LocationData){.scope = scope, .register_ = register_});
}

Location location_stack_slot(u32 scope, u32 stack) {
    return location_construct(LOCATION_STACK,
                              (LocationData){.scope = scope, .stack = stack});
}
