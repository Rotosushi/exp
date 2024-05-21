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

#include "backend/lifetimes.h"
#include "backend/x64_allocation.h"
#include "backend/x64_bytecode.h"
#include "backend/x64_gprp.h"

typedef struct X64ActiveAllocation {
  u16 ssa;
  Lifetime lifetime;
  X64Allocation allocation;
} X64ActiveAllocation;

typedef struct X64ActiveAllocations {
  u16 stack_size;
  u16 size;
  u16 capacity;
  X64ActiveAllocation *buffer;
} X64ActiveAllocations;

X64ActiveAllocations x64active_allocations_create();
void x64active_allocations_destroy(X64ActiveAllocations *restrict a);
X64ActiveAllocation
x64active_allocations_add(X64ActiveAllocations *restrict active,
                          u16 ssa,
                          Lifetime lifetime,
                          X64Allocation allocation);
void x64active_allocations_erase(X64ActiveAllocations *restrict a,
                                 X64ActiveAllocation *restrict aa);

/**
 * @brief manages where SSA locals are allocated
 *
 */
typedef struct X64Allocator {
  X64GPRP gprp;
  Lifetimes lifetimes;
  X64ActiveAllocations active;
  u16 stack_size;
} X64Allocator;

X64Allocator x64allocator_create(FunctionBody *restrict body);
void x64allocator_destroy(X64Allocator *restrict la);
u16 x64allocator_bump_active_stack_size(X64Allocator *restrict la);
void x64allocator_reduce_active_stack_size(X64Allocator *restrict la);
X64ActiveAllocation *x64allocator_allocation_of(X64Allocator *restrict la,
                                                u16 ssa);
X64ActiveAllocation *x64allocator_allocation_at(X64Allocator *restrict la,
                                                X64GPR gpr);

void x64allocator_release_expired_lifetimes(X64Allocator *restrict la, u16 Idx);
void x64allocator_release_gpr(X64Allocator *restrict allocator,
                              X64GPR gpr,
                              u16 Idx,
                              X64Bytecode *restrict x64bc);

void x64allocator_aquire_gpr(X64Allocator *restrict allocator,
                             X64GPR gpr,
                             u16 Idx,
                             X64Bytecode *restrict x64bc);

X64ActiveAllocation x64allocator_allocate(X64Allocator *restrict la,
                                          u16 Idx,
                                          u16 ssa,
                                          X64Bytecode *restrict x64bc);

X64ActiveAllocation
x64allocator_allocate_from_active(X64Allocator *restrict allocator,
                                  u16 Idx,
                                  u16 ssa,
                                  X64ActiveAllocation *active,
                                  X64Bytecode *restrict x64bc);

X64ActiveAllocation
x64allocator_allocate_to_gpr(X64Allocator *restrict allocator,
                             X64GPR gpr,
                             u16 Idx,
                             u16 ssa,
                             X64Bytecode *restrict x64bc);

void x64allocator_reallocate_active(X64Allocator *restrict allocator,
                                    X64ActiveAllocation *restrict active,
                                    X64Bytecode *restrict x64bc);

void x64allocator_spill_active(X64Allocator *restrict allocator,
                               X64ActiveAllocation *restrict allocation,
                               X64Bytecode *restrict x64bc);
X64GPR x64allocator_spill_oldest_active(X64Allocator *restrict allocator,
                                        X64Bytecode *restrict x64bc);

X64GPR x64allocator_aquire_any_gpr(X64Allocator *restrict allocator,
                                   u16 Idx,
                                   X64Bytecode *restrict x64bc);

#endif // !EXP_BACKEND_X64_ALLOCATOR_H