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
#ifndef EXP_BACKEND_REGISTER_ALLOCATOR_H
#define EXP_BACKEND_REGISTER_ALLOCATOR_H

#include "backend/register.h"

typedef enum AllocationKind {
  ALLOC_REG,
  ALLOC_STACK,
} AllocationKind;

typedef struct Allocation {
  u16 local;
  AllocationKind kind;
  union {
    Register reg;
    u16 offset;
  };
} Allocation;

typedef struct Allocations {
  u16 stack_size;
  u16 size;
  u16 capacity;
  Allocation *buffer;
} Allocations;

Allocations allocations_create();
void allocations_destroy(Allocations *restrict as);

/**
 * @brief allocate a local to the given register
 *
 * @param as
 * @param local
 * @param r
 */
void allocations_allocate(Allocations *restrict as, u16 local, Register r);

/**
 * @brief spill a new or existing local to the stack
 *
 * @param as
 * @param local
 */
void allocations_spill(Allocations *restrict as, u16 local);

#endif // !EXP_BACKEND_REGISTER_ALLOCATOR_H