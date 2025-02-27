// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_TARGETS_X86_64_IMR_ALLOCATION_H
#define EXP_TARGETS_X86_64_IMR_ALLOCATION_H

#include "imr/local.h"
#include "targets/x86_64/imr/address.h"

typedef enum x86_64_LocationKind {
    X86_64_LOCATION_UNINITIALIZED,
    X86_64_LOCATION_GENERAL_PURPOSE_REGISTER,
    X86_64_LOCATION_ADDRESS,
} x86_64_LocationKind;

typedef struct x86_64_Location {
    x86_64_LocationKind kind;
    union {
        x86_64_GPR gpr;
        x86_64_Address address;
    };
} x86_64_Location;

typedef struct x86_64_Allocation {
    Local local;
    x86_64_Location location;
} x86_64_Allocation;

#endif // EXP_TARGETS_X86_64_IMR_ALLOCATION_H
