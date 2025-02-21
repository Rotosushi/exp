// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file imr/location.h
 */

#ifndef EXP_IMR_LOCATION_H
#define EXP_IMR_LOCATION_H

#include "utility/int_types.h"

typedef enum LocationKind : u8 {
    LOCATION_UNINITIALIZED,
    LOCATION_REGISTER,
    LOCATION_STACK,
} LocationKind;

typedef struct LocationData {
    u32 scope;
    union {
        u8 register_;
        u32 stack;
    };
} LocationData;

typedef struct Location {
    LocationKind kind;
    LocationData data;
} Location;

Location location_uninitialized();
Location location_register(u32 scope, u8 register_);
Location location_stack_slot(u32 scope, u32 slot);

#endif // EXP_IMR_LOCATION_H
