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
#ifndef EXP_CODEGEN_X86_IMR_STACK_ALLOCATOR_H
#define EXP_CODEGEN_X86_IMR_STACK_ALLOCATOR_H

#include "codegen/x86/imr/value/function/allocation.h"

typedef enum x86_StackSpaceKind {
    X86_STACK_SPACE_KIND_ALLOCATION,
    X86_STACK_SPACE_KIND_PADDING,
} x86_StackSpaceKind;

typedef struct x86_StackSpaceData {
    i32 offset;
    union {
        x86_Allocation *allocation;
        i32             padding;
    };
} x86_StackSpaceData;

typedef struct x86_StackSpace {
    x86_StackSpaceKind kind;
    x86_StackSpaceData data;
} x86_StackSpace;

typedef struct x86_StackAllocator {
    i32             size;
    u32             length;
    u32             capacity;
    x86_StackSpace *buffer;
} x86_StackAllocator;

void x86_stack_allocator_create(x86_StackAllocator *restrict stack_allocator);

void x86_stack_allocator_destroy(x86_StackAllocator *restrict stack_allocator);

i32 x86_stack_allocator_stack_size(
    x86_StackAllocator *restrict stack_allocator);

void x86_stack_allocator_allocate_to_next_available(
    x86_StackAllocator *restrict stack_allocator,
    x86_Allocation *restrict allocation);

void x86_stack_allocator_release_expired(
    x86_StackAllocator *restrict stack_allocator, u32 block_index);

#endif // !EXP_CODEGEN_X86_IMR_STACK_ALLOCATOR_H
