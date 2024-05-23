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
#include "backend/x64/gprp.h"

// typedef struct X64ActiveAllocations {
//   u16 stack_size;
//   u16 size;
//   u16 capacity;
//   x64_Allocation *buffer;
// } X64ActiveAllocations;

// X64ActiveAllocations x64active_allocations_create();
// void x64active_allocations_destroy(X64ActiveAllocations *restrict a);
// x64_Allocation x64active_allocations_add(X64ActiveAllocations *restrict
// active,
//                                          u16 ssa,
//                                          Lifetime lifetime,
//                                          x64_Location location);
// void x64active_allocations_erase(X64ActiveAllocations *restrict a,
//                                  x64_Allocation *restrict aa);

typedef struct x64_StackAllocations {
  u16 active_stack_size;
  u16 total_stack_size;
  u16 count;
  u16 capacity;
  x64_Allocation **buffer;
} x64_StackAllocations;

x64_StackAllocations x64_stack_allocations_create();
void x64_stack_allocations_destroy(
    x64_StackAllocations *restrict stack_allocations);
void x64_stack_allocations_allocate(
    x64_StackAllocations *restrict stack_allocations,
    x64_Allocation *restrict allocation);
void x64_stack_allocations_erase(
    x64_StackAllocations *restrict stack_allocations,
    x64_Allocation *restrict allocation);
void x64_stack_allocations_release_expired_allocations(
    x64_StackAllocations *restrict stack_allocations, u16 Idx);
x64_Allocation *
x64_stack_allocations_of(x64_StackAllocations *restrict stack_allocations,
                         u16 ssa);

/**
 * @brief manages where SSA locals are allocated
 *
 */
typedef struct x64_Allocator {
  x64_GPRP gprp;
  x64_StackAllocations stack_allocations;
  Lifetimes lifetimes;
} x64_Allocator;

x64_Allocator x64_allocator_create(FunctionBody *restrict body);
void x64_allocator_destroy(x64_Allocator *restrict la);

bool x64_allocator_uses_stack(x64_Allocator *restrict allocator);
u16 x64_allocator_total_stack_size(x64_Allocator *restrict allocator);

x64_Allocation *x64_allocator_allocation_of(x64_Allocator *restrict la,
                                            u16 ssa);

void x64_allocator_release_gpr(x64_Allocator *restrict allocator,
                               x64_GPR gpr,
                               u16 Idx,
                               x64_Bytecode *restrict x64bc);

void x64_allocator_aquire_gpr(x64_Allocator *restrict allocator,
                              x64_GPR gpr,
                              u16 Idx,
                              x64_Bytecode *restrict x64bc);

x64_Allocation *x64_allocator_allocate(x64_Allocator *restrict la,
                                       u16 Idx,
                                       u16 ssa,
                                       x64_Bytecode *restrict x64bc);

x64_Allocation *
x64_allocator_allocate_from_active(x64_Allocator *restrict allocator,
                                   u16 Idx,
                                   u16 ssa,
                                   x64_Allocation *active,
                                   x64_Bytecode *restrict x64bc);

x64_Allocation *x64_allocator_allocate_to_gpr(x64_Allocator *restrict allocator,
                                              x64_GPR gpr,
                                              u16 Idx,
                                              u16 ssa,
                                              x64_Bytecode *restrict x64bc);

void x64_allocator_reallocate_active(x64_Allocator *restrict allocator,
                                     x64_Allocation *restrict active,
                                     x64_Bytecode *restrict x64bc);

x64_GPR x64_allocator_spill_oldest_active(x64_Allocator *restrict allocator,
                                          x64_Bytecode *restrict x64bc);

x64_GPR x64_allocator_aquire_any_gpr(x64_Allocator *restrict allocator,
                                     u16 Idx,
                                     x64_Bytecode *restrict x64bc);

#endif // !EXP_BACKEND_X64_ALLOCATOR_H