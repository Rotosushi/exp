// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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
    LOCATION_UNINITIALIZED,
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
Location location_register(u32 gpr);
Location location_stack_slot(u32 stack_slot);

#endif // EXP_IMR_LOCATION_H
