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
x64active_allocations_add(X64ActiveAllocations *restrict active,
                          u16 ssa,
                          Lifetime lifetime,
                          X64Allocation allocation) {
  if (x64active_allocations_full(active)) {
    x64active_allocations_grow(active);
  }

  // find the lifetime that ends later than the given lifetime
  // and insert before it.
  u16 i = 0;
  for (; i < active->size; ++i) {
    X64ActiveAllocation *al = active->buffer + i;
    if (al->lifetime.last_use > lifetime.last_use) { break; }
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
    if (al->ssa == aa->ssa) { break; }
  }

  // move all lifetimes after i backwards one location
  for (u16 j = i; j < a->size; ++j) {
    a->buffer[j] = a->buffer[j + 1];
  }

  a->size -= 1;
}

X64Allocator x64allocator_create(FunctionBody *restrict body) {
  X64Allocator la = {.gprp       = x64gprp_create(),
                     .lifetimes  = li_compute(body),
                     .active     = x64active_allocations_create(),
                     .stack_size = 0};
  // reserve the stack pointer RSP
  // and the frame pointer RBP
  // such that locals do not get allocated to them.
  x64gprp_force(&la.gprp, X64GPR_RSP);
  x64gprp_force(&la.gprp, X64GPR_RBP);
  return la;
}

void x64allocator_destroy(X64Allocator *restrict la) {
  x64gprp_destroy(&la->gprp);
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
    if (aa->ssa == ssa) { return aa; }
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

static X64ActiveAllocation *
x64allocator_oldest_active_gpr(X64Allocator *restrict allocator) {
  X64ActiveAllocation *oldest = NULL;
  u16 i                       = 0;
  // find the first active allocation in a gpr
  for (; i < allocator->active.size; ++i) {
    oldest = allocator->active.buffer + i;
    if (oldest->allocation.kind == ALLOC_GPR) { break; }
  }

  // find the active allocation in a gpr with the longest lifetime
  for (; i < allocator->active.size; ++i) {
    X64ActiveAllocation *cursor = allocator->active.buffer + i;
    if ((cursor->allocation.kind == ALLOC_GPR) &&
        (oldest->lifetime.last_use < cursor->lifetime.last_use)) {
      oldest = cursor;
    }
  }
  return oldest;
}

void x64allocator_release_expired_lifetimes(X64Allocator *restrict la,
                                            u16 Idx) {
  X64ActiveAllocations *a = &la->active;
  X64GPRP *gprp           = &la->gprp;
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
    if (al->lifetime.last_use >= Idx) { return; }

    if (al->allocation.kind == ALLOC_GPR) {
      x64gprp_release(gprp, al->allocation.gpr);
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

void x64allocator_release_gpr(X64Allocator *restrict allocator,
                              X64GPR gpr,
                              u16 Idx,
                              X64Bytecode *restrict x64bc) {
  X64ActiveAllocation *active = x64allocator_allocation_at(allocator, gpr);
  if (active == NULL) { return; }

  if (active->lifetime.last_use <= Idx) {
    x64gprp_release(&allocator->gprp, active->allocation.gpr);
    x64active_allocations_erase(&allocator->active, active);
    return;
  }

  x64allocator_reallocate_active(allocator, active, x64bc);
}

X64ActiveAllocation x64allocator_allocate(X64Allocator *restrict la,
                                          u16 Idx,
                                          u16 ssa,
                                          X64Bytecode *restrict x64bc) {
  X64GPR gpr         = 0;
  Lifetime *lifetime = li_at(&la->lifetimes, ssa);

  x64allocator_release_expired_lifetimes(la, Idx);

  if (x64gprp_allocate(&la->gprp, &gpr)) {
    return x64active_allocations_add(
        &la->active, ssa, *lifetime, x64allocation_reg(gpr));
  }

  // otherwise spill the oldest active allocation to the stack.
  X64ActiveAllocation *oldest_active = x64allocator_oldest_active_gpr(la);

  if (oldest_active->lifetime.last_use > lifetime->last_use) {
    X64GPR gpr = oldest_active->allocation.gpr;
    x64allocator_spill_active(la, oldest_active, x64bc);

    return x64active_allocations_add(
        &la->active, ssa, *lifetime, x64allocation_reg(gpr));
  }

  return x64active_allocations_add(
      &la->active,
      ssa,
      *lifetime,
      x64allocation_stack(x64allocator_bump_active_stack_size(la)));
}

/**
 * @brief allocate <ssa> and initialize it with the value of <active>
 * avoiding a copy if <active> expires at <Idx>
 *
 * @param allocator
 * @param Idx
 * @param ssa
 * @param active
 * @param x64bc
 * @return X64ActiveAllocation
 */
X64ActiveAllocation
x64allocator_allocate_from_active(X64Allocator *restrict allocator,
                                  u16 Idx,
                                  u16 ssa,
                                  X64ActiveAllocation *active,
                                  X64Bytecode *restrict x64bc) {
  Lifetime lifetime = *li_at(&allocator->lifetimes, ssa);

  if (active->lifetime.last_use <= Idx) {
    // we can reuse the existing allocation
    X64Allocation allocation = active->allocation;
    x64active_allocations_erase(&allocator->active, active);
    return x64active_allocations_add(
        &allocator->active, ssa, lifetime, allocation);
  }

  // we have to keep the existing allocation around
  X64ActiveAllocation new = x64allocator_allocate(allocator, Idx, ssa, x64bc);

  // initialize the new allocation
  if ((active->allocation.kind == ALLOC_STACK) &&
      (new.allocation.kind == ALLOC_STACK)) {
    X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64bytecode_append_mov(
        x64bc, x64opr_gpr(gpr), x64opr_stack(active->allocation.offset));
    x64bytecode_append_mov(
        x64bc, x64opr_stack(new.allocation.offset), x64opr_gpr(gpr));
  } else {
    x64bytecode_append_mov(x64bc,
                           x64opr_alloc(&new.allocation),
                           x64opr_alloc(&active->allocation));
  }

  return new;
}

X64ActiveAllocation
x64allocator_allocate_to_gpr(X64Allocator *restrict allocator,
                             X64GPR gpr,
                             u16 Idx,
                             u16 ssa,
                             X64Bytecode *restrict x64bc) {
  x64allocator_release_gpr(allocator, gpr, Idx, x64bc);

  x64gprp_force(&allocator->gprp, gpr);
  Lifetime lifetime = *li_at(&allocator->lifetimes, ssa);

  return x64active_allocations_add(
      &allocator->active, ssa, lifetime, x64allocation_reg(gpr));
}

void x64allocator_reallocate_active(X64Allocator *restrict allocator,
                                    X64ActiveAllocation *restrict active,
                                    X64Bytecode *restrict x64bc) {
  if (active->allocation.kind == ALLOC_STACK) { return; }

  X64GPR gpr = 0;
  if (x64gprp_allocate(&allocator->gprp, &gpr)) {
    x64bytecode_append_mov(
        x64bc, x64opr_gpr(gpr), x64opr_gpr(active->allocation.gpr));
    x64gprp_release(&allocator->gprp, active->allocation.gpr);
    active->allocation.gpr = gpr;
  } else {
    x64allocator_spill_active(allocator, active, x64bc);
  }
}

void x64allocator_spill_active(X64Allocator *restrict allocator,
                               X64ActiveAllocation *restrict allocation,
                               X64Bytecode *restrict x64bc) {
  X64GPR gpr = allocation->allocation.gpr;

  allocation->allocation.kind = ALLOC_STACK;
  allocation->allocation.offset =
      x64allocator_bump_active_stack_size(allocator);

  x64bytecode_append_mov(
      x64bc, x64opr_stack(allocation->allocation.offset), x64opr_gpr(gpr));
  x64gprp_release(&allocator->gprp, gpr);
}

X64GPR x64allocator_spill_oldest_active(X64Allocator *restrict allocator,
                                        X64Bytecode *restrict x64bc) {
  X64ActiveAllocation *oldest = x64allocator_oldest_active_gpr(allocator);
  X64GPR gpr                  = oldest->allocation.gpr;
  x64allocator_spill_active(allocator, oldest, x64bc);
  return gpr;
}

X64GPR x64allocator_aquire_any_gpr(X64Allocator *restrict allocator,
                                   u16 Idx,
                                   X64Bytecode *restrict x64bc) {
  x64allocator_release_expired_lifetimes(allocator, Idx);

  X64GPR gpr = 0;
  if (x64gprp_any_available(&allocator->gprp, &gpr)) { return gpr; }

  gpr = x64allocator_spill_oldest_active(allocator, x64bc);
  return gpr;
}