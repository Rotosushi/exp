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

#include "codegen/x86/imr/location.h"
#include "support/assert.h"

x86_Location x86_location_gpr(x86_GPR gpr) {
    return (x86_Location){.gpr = gpr, .is_address = false};
}

x86_Location x86_location_address(x86_GPR base, i32 offset) {
    return (x86_Location){
        .base = base, .has_index = false, .is_address = true, .offset = offset};
}

[[maybe_unused]] static bool valid_scale(u8 scale) {
    return (scale == 1) || (scale == 2) || (scale == 4) || (scale == 8);
}

x86_Location x86_location_address_indexed(x86_GPR base,
                                          x86_GPR index,
                                          u8      scale,
                                          i32     offset) {
    exp_assert(valid_scale(scale));
    return (x86_Location){.base       = base,
                          .index      = index,
                          .scale      = scale,
                          .has_index  = true,
                          .is_address = true,
                          .offset     = offset};
}

static bool x86_location_address_equality(x86_Location A, x86_Location B) {
    if (A.has_index && B.has_index) {
        return (A.base == B.base) && (A.index == B.index) &&
               (A.scale == B.scale) && (A.offset == B.offset);
    } else if (!A.has_index && !B.has_index) {
        return (A.base == B.base) && (A.offset == B.offset);
    } else {
        return false;
    }
}

bool x86_location_equality(x86_Location A, x86_Location B) {
    if (A.is_address && B.is_address) {
        return x86_location_address_equality(A, B);
    } else if (!A.is_address && !B.is_address) {
        return A.base == B.base;
    } else {
        return false;
    }
}

// effective address: [base + index * scale + offset]
static void print_x86_address(String *restrict buffer, x86_Location address) {
    string_append(buffer, SV("["));

    string_append(buffer, x86_gpr_mnemonic(address.base));

    if (address.has_index) {
        exp_assert(valid_scale(address.scale));
        string_append(buffer, SV(" + "));
        string_append(buffer, x86_gpr_mnemonic(address.index));
        string_append(buffer, SV(" * "));
        string_append_u64(buffer, address.scale);
    }

    if (address.offset < 0) {
        string_append(buffer, SV(" - "));
        string_append_i64(buffer, abs_i64(address.offset));
    } else if (address.offset > 0) {
        string_append(buffer, SV(" + "));
        string_append_i64(buffer, address.offset);
    }

    string_append(buffer, SV("]"));
}

void print_x86_location(String *restrict buffer, x86_Location location) {
    exp_assert(buffer != NULL);
    if (location.is_address) {
        print_x86_address(buffer, location);
        return;
    }

    string_append(buffer, x86_gpr_mnemonic(location.gpr));
}
