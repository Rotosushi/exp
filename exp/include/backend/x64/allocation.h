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
#ifndef EXP_BACKEND_X64_ALLOCATION_H
#define EXP_BACKEND_X64_ALLOCATION_H

#include "backend/lifetimes.h"
#include "backend/x64/location.h"
#include "imr/type.h"

typedef struct x64_Allocation {
    u64 ssa;
    Lifetime lifetime;
    x64_Location location;
    Type const *type;
} x64_Allocation;

x64_Allocation *x64_allocation_allocate();
void x64_allocation_deallocate(x64_Allocation *allocation);
bool x64_allocation_alive(x64_Allocation *allocation, u64 block_index);
bool x64_allocation_location_equality(x64_Allocation *allocation,
                                      x64_Location location);

#endif // !EXP_BACKEND_X64_ALLOCATION_H
