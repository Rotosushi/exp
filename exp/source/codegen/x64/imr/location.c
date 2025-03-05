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
#include <assert.h>
#include <stddef.h>

#include "codegen/x64/imr/location.h"
#include "utility/unreachable.h"

x64_Location x64_location_gpr(x64_GPR gpr) {
    x64_Location location = {.kind = LOCATION_GPR, .gpr = gpr};
    return location;
}

x64_Location x64_location_address(x64_GPR base,
                                  x64_GPR optional_index,
                                  u8 optional_scale,
                                  i64 optional_offset) {
    x64_Location location = {
        .kind    = LOCATION_ADDRESS,
        .address = {.base   = base,
                    .index  = optional_index,
                    .scale  = optional_scale,
                    .offset = optional_offset}
    };
    return location;
}

bool x64_location_eq(x64_Location A, x64_Location B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case LOCATION_GPR:     return A.gpr == B.gpr;
    case LOCATION_ADDRESS: return x64_address_equality(A.address, B.address);
    default:               EXP_UNREACHABLE();
    }
}
