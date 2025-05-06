// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X86_LOCATION_H
#define EXP_BACKEND_X86_LOCATION_H

#include "codegen/x86/imr/registers.h"
#include "support/string.h"

typedef struct x86_Location {
    union {
        x86_GPR base;
        x86_GPR gpr;
    };
    x86_GPR index;
    u8      scale;
    bool    has_index  : 1;
    bool    is_address : 1;
    i32     offset;
} x86_Location;

x86_Location x86_location_gpr(x86_GPR gpr);
x86_Location x86_location_address(x86_GPR base, i32 offset);
x86_Location
x86_location_address_indexed(x86_GPR base, x86_GPR index, u8 scale, i32 offset);

/**
 * @brief compares two locations for equality
 *
 * @note this does a lexicographic comparison,
 * and we do not use the values stored within the registers
 * (because we cannot) to confirm true equality. Thus this
 * method should be used with caution. if at all.
 */
bool x86_location_equality(x86_Location A, x86_Location B);

void print_x86_location(String *restrict buffer, x86_Location location);

#endif // !EXP_BACKEND_X86_LOCATION_H
