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
 * @file targets/x86_64/imr/address.c
 */

#include "targets/x86_64/imr/address.h"
#include "utility/assert.h"

[[maybe_unused]] static bool validate_scale(u8 scale) {
    return ((scale == 1) || (scale == 2) || (scale == 4) || (scale == 8));
}

x86_64_Address x86_64_address_create(x86_64_GPR base, i64 offset) {
    x86_64_Address address = {
        .base = base, .scale = 1, .has_index = false, .offset = offset};
    return address;
}

x86_64_Address x86_64_address_create_indexed(x86_64_GPR base, x86_64_GPR index,
                                             u8 scale, i64 offset) {
    EXP_ASSERT(validate_scale(scale));
    x86_64_Address address = {.base      = base,
                              .index     = index,
                              .scale     = scale,
                              .has_index = true,
                              .offset    = offset};
    return address;
}

void print_x86_64_address(String *buffer, x86_64_Address address) {
    EXP_ASSERT(buffer != nullptr);
    string_append_i64(buffer, address.offset);

    string_append(buffer, SV("("));
    print_x86_64_gpr(buffer, address.base);

    if (address.has_index) {
        string_append(buffer, SV(", "));
        print_x86_64_gpr(buffer, address.index);
        string_append(buffer, SV(", "));
        string_append_u64(buffer, address.scale);
    }

    string_append(buffer, SV(")"));
}
