/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef EXP_TARGETS_X86_64_LOCAL_ALLOCATOR_H
#define EXP_TARGETS_X86_64_LOCAL_ALLOCATOR_H

#include "targets/x86_64/imr/address.h"
#include "utility/bitset.h"

/*
 * #NOTE: The local allocator for the x86_64 target has two main tasks.
 *   keeping track of register allocations and keeping track of stack
 *   allocations. We know from the abstract allocator which locals
 *   are allocated in registers and which are allocated on the stack.
 *   however we need a mapping between abstract register slots and
 *   x86_64 registers. As well as a mapping between abstract stack slots
 *   and x86_64 stack slots.
 */

typedef struct x86_64_LocalAllocator {
    Bitset general_purpose_registers;
    i32 active_stack_size;
    i32 total_stack_size;
} x86_64_LocalAllocator;

#endif // EXP_TARGETS_X86_64_LOCAL_ALLOCATOR_H
