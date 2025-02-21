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
 * @file targets/x86_64/imr/allocation.h
 */

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
