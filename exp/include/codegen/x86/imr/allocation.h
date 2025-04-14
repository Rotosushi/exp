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
#ifndef EXP_BACKEND_X86_ALLOCATION_H
#define EXP_BACKEND_X86_ALLOCATION_H

#include "codegen/lifetimes.h"
#include "codegen/x86/imr/location.h"
#include "imr/type.h"

typedef struct x86_Allocation {
    u64          ssa;
    Lifetime     lifetime;
    x86_Location location;
    Type const  *type;
} x86_Allocation;

x86_Allocation *x86_allocation_allocate();
void            x86_allocation_deallocate(x86_Allocation *restrict allocation);

bool x86_allocation_location_eq(x86_Allocation *restrict allocation,
                                x86_Location location);

#endif // !EXP_BACKEND_X86_ALLOCATION_H
