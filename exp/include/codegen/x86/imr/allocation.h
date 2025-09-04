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

#include "codegen/x86/imr/allocation/location.h"
#include "imr/value/function/local.h"

typedef struct x86_Allocation {
    u32          ssa;
    unsigned     alive : 1;
    StringView   name;
    Lifetime     lifetime;
    x86_Location location;
    Type const  *type;
} x86_Allocation;

x86_Allocation *x86_allocation_allocate(Local const *restrict local);
void            x86_allocation_deallocate(x86_Allocation *restrict allocation);

bool x86_allocation_alive(x86_Allocation const *restrict allocation,
                          u32 block_index);

void x86_allocation_expire(x86_Allocation *restrict allocation);

u64 x86_allocation_size_of(x86_Allocation const *restrict allocation);
u64 x86_allocation_align_of(x86_Allocation const *restrict allocation);
x86_PtrKind
x86_allocation_ptr_kind_of(x86_Allocation const *restrict allocation);

x86_Location
x86_allocation_get_element_ptr(x86_Allocation const *restrict allocation,
                               u64 index);

#endif // !EXP_BACKEND_X86_ALLOCATION_H
