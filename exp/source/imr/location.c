/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
