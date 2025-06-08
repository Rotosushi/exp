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
#ifndef EXP_CODEGEN_X86_IMR_ALLOCATIONS_H
#define EXP_CODEGEN_X86_IMR_ALLOCATIONS_H

#include "codegen/x86/imr/allocation.h"

typedef struct x86_Allocations {
    u32              length;
    u32              capacity;
    x86_Allocation **buffer;
} x86_Allocations;

void x86_allocations_create(x86_Allocations *restrict allocations);

void x86_allocations_destroy(x86_Allocations *restrict allocations);

x86_Allocation *x86_allocations_append(x86_Allocations *restrict allocations,
                                       Local const *restrict local,
                                       Context *restrict context);

#endif // !EXP_CODEGEN_X86_IMR_ALLOCATIONS_H
