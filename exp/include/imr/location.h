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
#ifndef EXP_IMR_LOCATION_H
#define EXP_IMR_LOCATION_H

#include "utility/int_types.h"

// #NOTE: we use integer handles to refer to registers and stack slots.
//  since we are working with an abstract machine at this point in
//  the compilation process we are going to assume that we have
//  a u32's worth of general purpos registers, and a u32's worth
//  of stack slots. also, we are working with stack 'slots' and not
//  actual addresses, because again, this is programmed to the
//  abstract machine.

typedef enum LocationKind : u8 {
    LOCATION_EMPTY,
    LOCATION_GENERAL_PURPOSE_REGISTER,
    // #NOTE: does it makes sense to extend this structure life so:
    //  LOCATION_FLOATING_POINT_REGISTER,
    //  LOCATION_VECTOR_REGISTER,
    //  LOCATION_CONTEXT_REGISTER,
    LOCATION_STACK_SLOT,
} LocationKind;

typedef union LocationData {
    u32 general_purpose_register;
    u32 stack_slot;
} LocationData;

typedef struct Location {
    LocationKind kind;
    LocationData data;
} Location;

Location location_create();
Location location_general_purpose_register(u32 general_purpose_register);
Location location_stack_slot(u32 stack_slot);

#endif // EXP_IMR_LOCATION_H
