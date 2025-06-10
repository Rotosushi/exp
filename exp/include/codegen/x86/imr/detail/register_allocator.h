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
#ifndef EXP_CODEGEN_X86_IMR_REGISTER_ALLOCATOR_H
#define EXP_CODEGEN_X86_IMR_REGISTER_ALLOCATOR_H

#include "codegen/x86/imr/allocation.h"
#include "codegen/x86/imr/detail/register_pool.h"

typedef struct x86_RegisterAllocator {
    x86_RegisterPool pool;
    x86_Allocation **gpr_buffer;
} x86_RegisterAllocator;

void x86_register_allocator_create(
    x86_RegisterAllocator *restrict register_allocator);

void x86_register_allocator_destroy(
    x86_RegisterAllocator *restrict register_allocator);

void x86_register_allocator_aquire_gpr(
    x86_RegisterAllocator *restrict register_allocator, x86_GPR gpr);

void x86_register_allocator_release_gpr(
    x86_RegisterAllocator *restrict register_allocator, x86_GPR gpr);

bool x86_register_allocator_allocate_to_next_available(
    x86_RegisterAllocator *restrict register_allocator,
    x86_Allocation *restrict allocation);

void x86_register_allocator_release_expired(
    x86_RegisterAllocator *restrict register_allocator, u32 block_index);

#endif // !EXP_CODEGEN_X86_IMR_REGISTER_ALLOCATOR_H
