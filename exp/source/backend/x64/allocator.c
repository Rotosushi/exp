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
#include <assert.h>

#include "backend/x64/allocator.h"
#include "intrinsics/size_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/minmax.h"

x64_StackAllocations x64_stack_allocations_create() {
  x64_StackAllocations stack_allocations = {
      .active_stack_size = 0,
      .total_stack_size  = 0,
      .count             = 0,
      .capacity          = 0,
      .buffer            = NULL,
  };
  return stack_allocations;
}

void x64_stack_allocations_destroy(
    x64_StackAllocations *restrict stack_allocations) {
  if (stack_allocations->buffer != NULL) {
    for (u16 i = 0; i < 16; ++i) {
      deallocate(stack_allocations->buffer[i]);
    }
    deallocate(stack_allocations->buffer);
  }

  stack_allocations->count             = 0;
  stack_allocations->capacity          = 0;
  stack_allocations->active_stack_size = 0;
  stack_allocations->total_stack_size  = 0;
}

static bool
stack_allocations_full(x64_StackAllocations *restrict stack_allocations) {
  return (stack_allocations->count + 1) >= stack_allocations->capacity;
}

static void
stack_allocations_grow(x64_StackAllocations *restrict stack_allocations) {
  Growth g =
      array_growth_u16(stack_allocations->capacity, sizeof(x64_Allocation *));
  stack_allocations->buffer =
      reallocate(stack_allocations->buffer, g.alloc_size);
  stack_allocations->capacity = (u16)g.new_capacity;
}

void x64_stack_allocations_allocate(
    x64_StackAllocations *restrict stack_allocations,
    x64_Allocation *restrict allocation) {
  stack_allocations->active_stack_size +=
      (u16)ulmax(8UL, size_of(allocation->type));
  stack_allocations->total_stack_size =
      (u16)umax(stack_allocations->active_stack_size,
                stack_allocations->total_stack_size);

  if (stack_allocations_full(stack_allocations)) {
    stack_allocations_grow(stack_allocations);
  }

  allocation->location =
      x64_location_stack(stack_allocations->active_stack_size);
  stack_allocations->buffer[stack_allocations->count] = allocation;
  stack_allocations->count += 1;
}

void x64_stack_allocations_erase(
    x64_StackAllocations *restrict stack_allocations,
    x64_Allocation *restrict allocation) {
  u16 i = 0;
  for (; i < stack_allocations->count; ++i) {
    x64_Allocation *cursor = stack_allocations->buffer[i];
    if (cursor == allocation) { break; }
  }

  if (i == stack_allocations->count) { return; }

  for (; i < stack_allocations->count; ++i) {
    stack_allocations->buffer[i] = stack_allocations->buffer[i + 1];
  }

  stack_allocations->count -= 1;
}

void x64_stack_allocations_release_expired_allocations(
    x64_StackAllocations *restrict stack_allocations, u16 Idx) {
  for (u16 i = 0; i < stack_allocations->count; ++i) {
    x64_Allocation *cursor = stack_allocations->buffer[i];
    if (cursor->lifetime.last_use <= Idx) {
      x64_stack_allocations_erase(stack_allocations, cursor);
      --i;
    }
  }
}

x64_Allocation *
x64_stack_allocations_of(x64_StackAllocations *restrict stack_allocations,
                         u16 ssa) {
  for (u16 i = 0; i < stack_allocations->count; ++i) {
    x64_Allocation *cursor = stack_allocations->buffer[i];
    if (cursor->ssa == ssa) { return cursor; }
  }
  return NULL;
}

x64_Allocator x64_allocator_create(FunctionBody *restrict body) {
  x64_Allocator allocator = {
      .gprp              = x64_gprp_create(),
      .stack_allocations = x64_stack_allocations_create(),
      .lifetimes         = li_compute(body),
  };
  x64_gprp_aquire(&allocator.gprp, X64GPR_RSP);
  x64_gprp_aquire(&allocator.gprp, X64GPR_RBP);
  return allocator;
}

void x64_allocator_destroy(x64_Allocator *restrict allocator) {
  x64_gprp_destroy(&allocator->gprp);
  x64_stack_allocations_destroy(&allocator->stack_allocations);
  li_destroy(&allocator->lifetimes);
}

bool x64_allocator_uses_stack(x64_Allocator *restrict allocator) {
  return allocator->stack_allocations.total_stack_size > 0;
}

u16 x64_allocator_total_stack_size(x64_Allocator *restrict allocator) {
  return allocator->stack_allocations.total_stack_size;
}

static void
x64_allocator_release_expired_lifetimes(x64_Allocator *restrict allocator,
                                        u16 Idx) {
  x64_stack_allocations_release_expired_allocations(
      &allocator->stack_allocations, Idx);
  x64_gprp_release_expired_allocations(&allocator->gprp, Idx);
}

static void x64_allocator_spill_allocation(x64_Allocator *restrict allocator,
                                           x64_Allocation *restrict allocation,
                                           x64_Bytecode *restrict x64bc) {
  assert(allocation->location.kind == ALLOC_GPR);
  x64_GPR gpr = allocation->location.gpr;
  x64_gprp_release(&allocator->gprp, gpr);
  x64_stack_allocations_allocate(&allocator->stack_allocations, allocation);

  x64_bytecode_append_mov(
      x64bc, x64_opr_stack(allocation->location.offset), x64_opr_gpr(gpr));
}

x64_Allocation *x64_allocator_allocation_of(x64_Allocator *restrict allocator,
                                            u16 ssa) {
  x64_Allocation *allocation = x64_gprp_allocation_of(&allocator->gprp, ssa);
  if (allocation != NULL) { return allocation; }

  return x64_stack_allocations_of(&allocator->stack_allocations, ssa);
}

void x64_allocator_release_gpr(x64_Allocator *restrict allocator,
                               x64_GPR gpr,
                               u16 Idx,
                               x64_Bytecode *restrict x64bc) {
  x64_Allocation *active = x64_gprp_allocation_at(&allocator->gprp, gpr);
  if ((active == NULL) || active->lifetime.last_use <= Idx) {
    x64_gprp_release(&allocator->gprp, gpr);
    return;
  }

  x64_allocator_reallocate_active(allocator, active, x64bc);
}

void x64_allocator_aquire_gpr(x64_Allocator *restrict allocator,
                              x64_GPR gpr,
                              u16 Idx,
                              x64_Bytecode *restrict x64bc) {
  x64_Allocation *active = x64_gprp_allocation_at(&allocator->gprp, gpr);
  if (active == NULL) {
    x64_gprp_aquire(&allocator->gprp, gpr);
    return;
  }

  if (active->lifetime.last_use <= Idx) {
    x64_gprp_release(&allocator->gprp, gpr);
    x64_gprp_aquire(&allocator->gprp, gpr);
    return;
  }

  x64_allocator_reallocate_active(allocator, active, x64bc);
}

static x64_Allocation *
x64_allocation_allocate(u16 ssa, Lifetime *lifetime, Type *type) {
  x64_Allocation *allocation = allocate(sizeof(x64_Allocation));
  allocation->ssa            = ssa;
  allocation->lifetime       = *lifetime;
  allocation->type           = type;
  return allocation;
}

x64_Allocation *x64_allocator_allocate(x64_Allocator *restrict la,
                                       u16 Idx,
                                       LocalVariable *local,
                                       x64_Bytecode *restrict x64bc) {
  /*
    #TODO: in order to support assignment we are going to have to allocate
    local variables into a stack slot. this has a lot of knock on effects.
    for one the way we select instructions might need to change to be more
    aware of this fact.
    for another we need some way of associating a local variable with a stack
    allocation, and a register allocation, and being able to mark the register
    allocation as out of date, and the stack allocation as out of date.
    (the register allocation acts sort of like a cache for the stack allocated
     value.)

  */
  Lifetime *lifetime = li_at(&la->lifetimes, local->ssa);
  x64_Allocation *allocation =
      x64_allocation_allocate(local->ssa, lifetime, local->type);

  x64_allocator_release_expired_lifetimes(la, Idx);

  if (x64_gprp_allocate(&la->gprp, allocation)) { return allocation; }

  // otherwise spill the oldest active allocation to the stack.
  x64_Allocation *oldest_active = x64_gprp_oldest_allocation(&la->gprp);

  if (oldest_active->lifetime.last_use > lifetime->last_use) {
    x64_allocator_spill_allocation(la, oldest_active, x64bc);
    x64_gprp_allocate(&la->gprp, allocation);
  } else {
    x64_stack_allocations_allocate(&la->stack_allocations, allocation);
  }

  return allocation;
}

/**
 * @brief allocate <ssa> in the same location as <active>
 * reallocating <active> if <active> expires after <Idx>
 *
 */
x64_Allocation *
x64_allocator_allocate_from_active(x64_Allocator *restrict allocator,
                                   u16 Idx,
                                   LocalVariable *local,
                                   x64_Allocation *active,
                                   x64_Bytecode *restrict x64bc) {
  Lifetime *lifetime = li_at(&allocator->lifetimes, local->ssa);

  if (active->lifetime.last_use <= Idx) {
    // we can reuse the existing allocation treating
    // it as the new ssa local allocation.
    active->ssa      = local->ssa;
    active->lifetime = *lifetime;
    return active;
  }

  // we have to keep the existing allocation around
  x64_Allocation *new = x64_allocator_allocate(allocator, Idx, local, x64bc);

  // initialize the new allocation
  if ((active->location.kind == ALLOC_STACK) &&
      (new->location.kind == ALLOC_STACK)) {
    x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(gpr), x64_opr_stack(active->location.offset));
    x64_bytecode_append_mov(
        x64bc, x64_opr_stack(new->location.offset), x64_opr_gpr(gpr));
  } else {
    x64_bytecode_append_mov(x64bc, x64_opr_alloc(new), x64_opr_alloc(active));
  }

  return new;
}

x64_Allocation *x64_allocator_allocate_to_gpr(x64_Allocator *restrict allocator,
                                              x64_GPR gpr,
                                              u16 Idx,
                                              LocalVariable *local,
                                              x64_Bytecode *restrict x64bc) {
  x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);

  Lifetime *lifetime = li_at(&allocator->lifetimes, local->ssa);
  x64_Allocation *allocation =
      x64_allocation_allocate(local->ssa, lifetime, local->type);

  x64_gprp_allocate_to_gpr(&allocator->gprp, gpr, allocation);
  return allocation;
}

void x64_allocator_reallocate_active(x64_Allocator *restrict allocator,
                                     x64_Allocation *restrict active,
                                     x64_Bytecode *restrict x64bc) {
  if (active->location.kind == ALLOC_STACK) { return; }

  x64_GPR prev_gpr = active->location.gpr;
  if (x64_gprp_reallocate(&allocator->gprp, active)) {
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(active->location.gpr), x64_opr_gpr(prev_gpr));
  } else {
    x64_allocator_spill_allocation(allocator, active, x64bc);
  }
}

x64_GPR x64_allocator_spill_oldest_active(x64_Allocator *restrict allocator,
                                          x64_Bytecode *restrict x64bc) {
  x64_Allocation *oldest = x64_gprp_oldest_allocation(&allocator->gprp);
  x64_GPR gpr            = oldest->location.gpr;
  x64_allocator_spill_allocation(allocator, oldest, x64bc);
  return gpr;
}

x64_GPR x64_allocator_aquire_any_gpr(x64_Allocator *restrict allocator,
                                     u16 Idx,
                                     x64_Bytecode *restrict x64bc) {
  x64_allocator_release_expired_lifetimes(allocator, Idx);

  x64_GPR gpr = 0;
  if (x64_gprp_any_available(&allocator->gprp, &gpr)) { return gpr; }

  gpr = x64_allocator_spill_oldest_active(allocator, x64bc);
  return gpr;
}