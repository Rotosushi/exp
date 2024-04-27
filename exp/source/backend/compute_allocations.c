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
#include <stddef.h>
#include <stdlib.h>

#include "backend/compute_allocations.h"
#include "backend/register_set.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

/*
The target algorithm is Linear Scan Register Allocation (LSRA)
The goal of the algorithm is to allocate the local variables
and temporary variables within a function to specific registers,
such that the function body can be mapped to target assembly
instructions.

in pseudo-code:

LinearScanRegisterAllocation
    active ← {}
    for each live interval i, in order of increasing start point do
        ExpireOldIntervals(i)
        if length(active) = R then
            SpillAtInterval(i)
        else
            register[i] ← a register removed from pool of free registers
            add i to active, sorted by increasing end point

ExpireOldIntervals(i)
    for each interval j in active, in order of increasing end point do
        if endpoint[j] ≥ startpoint[i] then
            return
        remove j from active
        add register[j] to pool of free registers

SpillAtInterval(i)
    spill ← last interval in active
    if endpoint[spill] > endpoint[i] then
        register[i] ← register[spill]
        location[spill] ← new stack location
        remove spill from active
        add i to active, sorted by increasing end point
    else
        location[i] ← new stack location

-----

  infrastructure to build
    - a pool of registers which can be marked active/inactive
    - a pool of stack space, where SSA locals can be spilled/filled
    - a liveness range for each SSA local
    - a list of currently active liveness ranges
    - a mapping between SSA local and Register

  The LSRA needs to take into account a few more things:
    - function arguments have defined registers which they take up,
      which affect the location a SSA local must be in, in the future.
    - certain instructions on the target architecture have defined
      registers which are their operands, which affects the location
      of the corresponding SSA locals
  This can be done by a pre-allocation step, which runs before
  the main allocation algorithm.
*/
typedef struct ActiveLifetimes {
  u8 size;
  u8 capacity;
  Lifetime *buffer;
} ActiveLifetimes;

static ActiveLifetimes active_lifetimes_create() {
  ActiveLifetimes al = {.size = 0, .capacity = 0, .buffer = NULL};
  return al;
}

static void active_lifetimes_destroy(ActiveLifetimes *restrict al) {
  al->size     = 0;
  al->capacity = 0;
  free(al->buffer);
  al->buffer = NULL;
}

static bool active_lifetimes_full(ActiveLifetimes *restrict al) {
  return (al->size + 1) >= al->capacity;
}

static void active_lifetimes_grow(ActiveLifetimes *restrict al) {
  Growth g     = array_growth_u8(al->capacity, sizeof(Lifetime));
  al->buffer   = reallocate(al->buffer, g.alloc_size);
  al->capacity = (u8)g.new_capacity;
}

// we sort active lifetimes by smallest end point.
// (in a sense, longest lifetime)
static void active_lifetimes_insert_sorted(ActiveLifetimes *restrict al,
                                           Lifetime l) {
  if (active_lifetimes_full(al)) {
    active_lifetimes_grow(al);
  }

  // find the first lifetime that ends later than the
  // given lifetime and insert before it.
  u8 i = 0;
  for (; i < al->size; ++i) {
    if (al->buffer[i].last_use > l.last_use) {
      break;
    }
  }

  // shift all lifetimes after i forward one location
  for (u8 j = al->size; j > i; --j) {
    al->buffer[j] = al->buffer[j - 1];
  }

  al->buffer[i] = l;
  al->size += 1;
}

static void active_lifetimes_remove(ActiveLifetimes *restrict al, Lifetime l) {
  // find the index <i> of the given lifetime <l>
  u8 i = 0;
  for (; i < al->size; ++i) {
    if (al->buffer[i].local == l.local) {
      break;
    }
  }

  // shift all lifetimes after <i> backward one location
  for (u8 j = i; j < al->size; ++j) {
    al->buffer[j] = al->buffer[j + 1];
  }

  al->size -= 1;
}

typedef struct Allocator {
  ActiveLifetimes active;
  RegisterSet rs;
} Allocator;

static Allocator allocator_create() {
  Allocator a = {.active = active_lifetimes_create(),
                 .rs     = register_set_create()};
  return a;
}

static void allocator_destroy(Allocator *restrict a) {
  active_lifetimes_destroy(&a->active);
  a->rs = register_set_create();
}

static Lifetime last_active(Allocator *restrict a) {
  // generally speaking we would have to handle the
  // case where the list was empty. However we
  // know as a precondition to calling this function
  // that we are in the process of spilling an allocation
  // to the stack, which can only happen when all
  // available registers have been allocated to the
  // currently active lifetimes. Thus there must
  // be a number of active lifetimes equal to the number
  // of available registers.
  return a->active.buffer[a->active.size - 1];
}

// release the register holding <l>
// remove <l> from active lifetimes
// spill <l>'s allocation to the stack
// increase the current stack size.
static Register spill_active_lifetime(Allocator *restrict ar,
                                      Allocations *restrict al, Lifetime l) {
  Register r = register_set_release(&ar->rs, l.local);
  active_lifetimes_remove(&ar->active, l);
  allocations_spill(al, l.local);
  return r;
}

/*
SpillAtInterval(i)
    spill ← last interval in active
    if endpoint[spill] > endpoint[i] then
        register[i] ← register[spill]
        location[spill] ← new stack location
        remove spill from active
        add i to active, sorted by increasing end point
    else
        location[i] ← new stack location

in my own words:
  of the currently active lifetimes spill the lifetime that ends last.

  (in the papers own words "..there are other heuristics available...")

  since active lifetimes is sorted in order of increasing end point,
  the last lifetime in the active list has the largest end point.

  We are at a point where a new lifetime is beginning, l.
  and l could end later than the last active lifetime.

  so we see which lifetime ends later, and spill that one to
  the stack.

*/
static void spill_at_interval(Allocator *restrict ar, Allocations *restrict al,
                              Lifetime l) {
  Lifetime spill = last_active(ar);

  if (spill.last_use > l.last_use) {
    Register r = spill_active_lifetime(ar, al, spill);
    // allocate l in the spilled register
    allocations_allocate(al, l.local, r);
    active_lifetimes_insert_sorted(&ar->active, l);
  } else {
    // spill l
    allocations_spill(al, l.local);
  }
}

/*
ExpireOldIntervals(i)
    for each interval j in active, in order of increasing end point do
        if endpoint[j] ≥ startpoint[i] then
            return
        remove j from active
        add register[j] to pool of free registers

in my own words:
  remove from the active lifetimes all lifetimes
  which end earlier than the current lifetime begins.
*/
static void expire_old_intervals(Allocator *restrict a, Lifetime l) {
  for (u16 i = 0; i < a->active.size; ++i) {
    Lifetime j = a->active.buffer[i];
    // since the last_uses increase as we walk the list of
    // active lifetimes, once we see a lifetime which ends
    // after the current lifetime begins, the rest of
    // the active lifetimes do as well. Which means
    // the rest of the active lifetimes are going to
    // be active during the current lifetime.
    if (j.last_use >= l.first_use) {
      return;
    }
    active_lifetimes_remove(&a->active, j);
    register_set_release(&a->rs, j.local);
  }
}

/*
LinearScanRegisterAllocation
    active ← {}
    for each live interval i, in order of increasing start point do
        ExpireOldIntervals(i)
        if length(active) = R then
            SpillAtInterval(i)
        else
            register[i] ← a register removed from pool of free registers
            add i to active, sorted by increasing end point

in my own words:
  for each lifetime
    remove active lifetimes which are over

    if there is a register available for the current
    lifetime allocate it
    else
    spill an active lifetime.
*/
static void linear_scan(Allocator *restrict ar, Allocations *restrict as,
                        Lifetimes *restrict ls) {
  for (u16 i = 0; i < ls->size; ++i) {
    Lifetime l = ls->buffer[i];

    expire_old_intervals(ar, l);

    Register r = register_set_allocate(&ar->rs, l.local);
    if (r == REG_NONE) {
      spill_at_interval(ar, as, l);
    } else {
      allocations_allocate(as, l.local, r);
      active_lifetimes_insert_sorted(&ar->active, l);
    }
  }
}

//
// static void compute_preallocations(Allocator *restrict ar,
// Allocations *restrict as,
// Lifetimes *restrict ls) {
// what needs to be preallocated?
// well, in general, local variables which
// are used by specific instructions.
//

//}

Allocations compute_allocations(Lifetimes *restrict ls) {
  Allocations as = allocations_create();
  Allocator ar   = allocator_create();

  linear_scan(&ar, &as, ls);

  allocator_destroy(&ar);
  return as;
}