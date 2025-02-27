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
#ifndef EXP_BACKEND_X64_ALLOCATOR_H
#define EXP_BACKEND_X64_ALLOCATOR_H

#include "backend/x64/allocation.h"
#include "backend/x64/bytecode.h"
#include "backend/x64/registers.h"

/**
 * @brief General Purpose Register Pool
 *
 */
typedef struct x64_GPRP {
  u16 bitset;
  x64_Allocation **buffer;
} x64_GPRP;

typedef struct x64_StackAllocations {
  i64 active_stack_size;
  i64 total_stack_size;
  u64 count;
  u64 capacity;
  x64_Allocation **buffer;
} x64_StackAllocations;

typedef struct x64_AllocationBuffer {
  u64 count;
  u64 capacity;
  x64_Allocation **buffer;
} x64_AllocationBuffer;

/**
 * @brief manages where SSA locals are allocated
 *
 */
typedef struct x64_Allocator {
  x64_GPRP gprp;
  x64_StackAllocations stack_allocations;
  x64_AllocationBuffer allocations;
  Lifetimes lifetimes;
} x64_Allocator;

x64_Allocator x64_allocator_create(FunctionBody *restrict body,
                                   Context *restrict context);
void x64_allocator_destroy(x64_Allocator *restrict allocator);

bool x64_allocator_uses_stack(x64_Allocator *restrict allocator);
i64 x64_allocator_total_stack_size(x64_Allocator *restrict allocator);

x64_Allocation *x64_allocator_allocation_of(x64_Allocator *restrict allocator,
                                            u64 ssa);

void x64_allocator_release_gpr(x64_Allocator *restrict allocator,
                               x64_GPR gpr,
                               u64 Idx,
                               x64_Bytecode *restrict x64bc);

void x64_allocator_aquire_gpr(x64_Allocator *restrict allocator,
                              x64_GPR gpr,
                              u64 Idx,
                              x64_Bytecode *restrict x64bc);

x64_Allocation *x64_allocator_allocate(x64_Allocator *restrict allocator,
                                       u64 Idx,
                                       LocalVariable *local,
                                       x64_Bytecode *restrict x64bc);

x64_Allocation *
x64_allocator_allocate_from_active(x64_Allocator *restrict allocator,
                                   u64 Idx,
                                   LocalVariable *local,
                                   x64_Allocation *active,
                                   x64_Bytecode *restrict x64bc);

x64_Allocation *x64_allocator_allocate_to_gpr(x64_Allocator *restrict allocator,
                                              x64_GPR gpr,
                                              u64 Idx,
                                              LocalVariable *local,
                                              x64_Bytecode *restrict x64bc);

x64_Allocation *x64_allocator_allocate_to_stack(
    x64_Allocator *restrict allocator, i64 offset, LocalVariable *local);

x64_Allocation *x64_allocator_allocate_result(x64_Allocator *restrict allocator,
                                              x64_Location location,
                                              Type *type);

void x64_allocator_reallocate_active(x64_Allocator *restrict allocator,
                                     x64_Allocation *restrict active,
                                     x64_Bytecode *restrict x64bc);

x64_GPR x64_allocator_spill_oldest_active(x64_Allocator *restrict allocator,
                                          x64_Bytecode *restrict x64bc);

x64_GPR x64_allocator_aquire_any_gpr(x64_Allocator *restrict allocator,
                                     u64 Idx,
                                     x64_Bytecode *restrict x64bc);

#endif // !EXP_BACKEND_X64_ALLOCATOR_H
