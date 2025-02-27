// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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
