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
#ifndef EXP_IMR_ALLOCATOR_H
#define EXP_IMR_ALLOCATOR_H

#include "imr/local.h"
#include "utility/bitset.h"

typedef struct Locals {
    u32 count;
    u32 capacity;
    Local *buffer;
} Locals;

typedef struct LocalAllocator {
    Bitset registers;
    u32 stack_slots;
    Locals locals;
} LocalAllocator;

void local_allocator_initialize(LocalAllocator *allocator);
void local_allocator_terminate(LocalAllocator *allocator);

u32 local_allocator_declare_ssa(LocalAllocator *allocator);
Local *local_allocator_at(LocalAllocator *allocator, u32 ssa);
// Local *local_allocator_at_name(LocalAllocator *allocator, StringView name);
/*
void local_allocator_allocate_result(LocalAllocator *allocator, Local *local);
void local_allocator_allocate_formal_argument(LocalAllocator *allocator,
                                              Local *local,
                                              u8 argument_index);
void local_allocator_allocate_actual_argument(LocalAllocator *allocator,
                                              Local *local,
                                              u8 argument_index,
                                              u32 block_index);
*/
void local_allocator_allocate_local(LocalAllocator *allocator,
                                    Local *local,
                                    u32 block_index);

#endif // EXP_IMR_ALLOCATOR_H
