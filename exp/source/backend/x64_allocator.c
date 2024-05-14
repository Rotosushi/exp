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

#include "backend/x64_allocator.h"

#include "utility/alloc.h"
#include "utility/array_growth.h"

X64Allocation x64allocation_reg(X64GPR gpr) {
  X64Allocation a = {.kind = ALLOC_GPR, .gpr = gpr};
  return a;
}

X64Allocation x64allocation_stack(u16 offset) {
  X64Allocation a = {.kind = ALLOC_STACK, .offset = offset};
  return a;
}

X64ActiveAllocations x64active_allocations_create() {
  X64ActiveAllocations a = {
      .stack_size = 0, .size = 0, .capacity = 0, .buffer = NULL};
  return a;
}

void x64active_allocations_destroy(X64ActiveAllocations *restrict a) {
  a->stack_size = 0;
  a->size       = 0;
  a->capacity   = 0;
  free(a->buffer);
  a->buffer = NULL;
}

static bool x64active_allocations_full(X64ActiveAllocations *restrict a) {
  return (a->size + 1) >= a->capacity;
}

static void x64active_allocations_grow(X64ActiveAllocations *restrict a) {
  Growth g    = array_growth_u16(a->capacity, sizeof(X64ActiveAllocation));
  a->buffer   = reallocate(a->buffer, g.alloc_size);
  a->capacity = (u16)g.new_capacity;
}

X64ActiveAllocation
x64active_allocations_add(X64ActiveAllocations *restrict active, u16 ssa,
                          Lifetime lifetime, X64Allocation allocation) {
  if (x64active_allocations_full(active)) {
    x64active_allocations_grow(active);
  }

  // find the lifetime that ends later than the given lifetime
  // and insert before it.
  u16 i = 0;
  for (; i < active->size; ++i) {
    X64ActiveAllocation *al = active->buffer + i;
    if (al->lifetime.last_use > lifetime.last_use) {
      break;
    }
  }

  // shift all lifetimes after idx forward one location
  for (u16 j = active->size; j > i; --j) {
    active->buffer[j] = active->buffer[j - 1];
  }

  X64ActiveAllocation *aa = active->buffer + i;
  active->size += 1;

  *aa = (X64ActiveAllocation){
      .ssa = ssa, .lifetime = lifetime, .allocation = allocation};
  return *aa;
}

void x64active_allocations_erase(X64ActiveAllocations *restrict a,
                                 X64ActiveAllocation *restrict aa) {
  u16 i = 0;
  for (; i < a->size; ++i) {
    X64ActiveAllocation *al = a->buffer + i;
    if (al->ssa == aa->ssa) {
      break;
    }
  }

  // move all lifetimes after i backwards one location
  for (u16 j = i; j < a->size; ++j) {
    a->buffer[j] = a->buffer[j + 1];
  }

  a->size -= 1;
}

X64Allocator x64allocator_create(FunctionBody *restrict body) {
  X64Allocator la = {.gprp       = gprp_create(),
                     .lifetimes  = li_compute(body),
                     .active     = x64active_allocations_create(),
                     .stack_size = 0};
  // reserve the stack pointer RSP
  // and the frame pointer RBP
  // such that locals do not get allocated to them.
  gprp_force(&la.gprp, X64GPR_RSP);
  gprp_force(&la.gprp, X64GPR_RBP);
  return la;
}

void x64allocator_destroy(X64Allocator *restrict la) {
  gprp_destroy(&la->gprp);
  li_destroy(&la->lifetimes);
  x64active_allocations_destroy(&la->active);
  la->stack_size = 0;
}

u16 x64allocator_bump_active_stack_size(X64Allocator *restrict la) {
  // #TODO: as a simplification we just use a full word to store each
  // local on the stack.
  la->active.stack_size += 8;

  if (la->stack_size < la->active.stack_size) {
    la->stack_size = la->active.stack_size;
  }
  return la->active.stack_size;
}

void x64allocator_reduce_active_stack_size(X64Allocator *restrict la) {
  // #TODO similarly to the total stack size we don't take into
  // account the size of the elements stored on the stack.
  la->active.stack_size -= 8;
}

X64ActiveAllocation *x64allocator_allocation_of(X64Allocator *restrict la,
                                                u16 ssa) {
  X64ActiveAllocations *active = &la->active;
  for (u16 i = 0; i < active->size; ++i) {
    X64ActiveAllocation *aa = active->buffer + i;
    if (aa->ssa == ssa) {
      return aa;
    }
  }
  return NULL;
}

X64ActiveAllocation *x64allocator_allocation_at(X64Allocator *restrict la,
                                                X64GPR gpr) {
  X64ActiveAllocations *active = &la->active;
  for (u16 i = 0; i < active->size; ++i) {
    X64ActiveAllocation *aa = active->buffer + i;
    if ((aa->allocation.kind == ALLOC_GPR) && (aa->allocation.gpr == gpr)) {
      return aa;
    }
  }
  return NULL;
}

void x64allocator_expire_old_lifetimes(X64Allocator *restrict la, u16 Idx) {
  X64ActiveAllocations *a = &la->active;
  GPRP *gprp              = &la->gprp;
  u16 end                 = a->size;
  for (u16 i = 0; i < end; ++i) {
    // copy the active lifetime
    X64ActiveAllocation *al = a->buffer + i;

    // since we store active lifetimes in order
    // of increasing last_use, if we find an
    // active lifetime which ends later than
    // the current instruction (Idx), we know that the rest
    // of the active lifetimes also end later.
    // thus we can exit early.
    if (al->lifetime.last_use >= Idx) {
      return;
    }

    if (al->allocation.kind == ALLOC_GPR) {
      gprp_release(gprp, al->allocation.gpr);
    } else {
      x64allocator_reduce_active_stack_size(la);
    }

    x64active_allocations_erase(a, al);

    // since we remove an element from active lifetimes
    // we also have to update the end point, so we don't
    // read garbage. I think this is okay because
    // A - we don't try to access a lifetime after it is
    //  removed
    // B - we recompute the pointer on each iteration
    // C - ActiveLifetimes keeps all elements stored
    //  contiguously.
    // D - since we decrement the end by one when we remove
    //  one element, end stays lock-step with the size of
    //  ActiveLifetimes.
    // E - removing elements will never trigger a reallocation
    //  of the memory allocated for the buffer of active lifetimes.
    end -= 1;
    // since we removed an element, the next active lifetime
    // will be at the current index. so we update our index
    // here to account for that.
    i -= 1;
  }
}

X64ActiveAllocation x64allocator_allocate(X64Allocator *restrict la, u16 Idx,
                                          u16 ssa) {
  X64GPR gpr         = 0;
  Lifetime *lifetime = li_at(&la->lifetimes, ssa);

  x64allocator_expire_old_lifetimes(la, Idx);

  if (gprp_allocate(&la->gprp, &gpr)) {
    return x64active_allocations_add(&la->active, ssa, *lifetime,
                                     x64allocation_reg(gpr));
  }

  // otherwise spill to the stack.
  return x64active_allocations_add(
      &la->active, ssa, *lifetime,
      x64allocation_stack(x64allocator_bump_active_stack_size(la)));
}