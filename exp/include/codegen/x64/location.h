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
#ifndef EXP_BACKEND_X64_LOCATION_H
#define EXP_BACKEND_X64_LOCATION_H

#include "codegen/x64/address.h"
#include "codegen/x64/registers.h"

typedef enum x64_LocationKind : u8 {
    LOCATION_GPR,
    LOCATION_ADDRESS,
} x64_LocationKind;

typedef struct x64_Location {
    x64_LocationKind kind;
    union {
        x64_GPR gpr;
        x64_Address address;
    };
} x64_Location;

x64_Location x64_location_gpr(x64_GPR gpr);
x64_Location
x64_location_address(x64_GPR base, x64_GPR index, u8 scale, i64 offset);
bool x64_location_eq(x64_Location A, x64_Location B);

#endif // !EXP_BACKEND_X64_LOCATION_H
