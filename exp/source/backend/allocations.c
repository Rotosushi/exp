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

#include "backend/allocations.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/panic.h"

static Allocation register_(u16 local, Register reg) {
  Allocation a = {.local = local, .kind = ALLOC_REG, .reg = reg};
  return a;
}

static Allocation stack(u16 local, u16 offset) {
  Allocation a = {.local = local, .kind = ALLOC_STACK, .offset = offset};
  return a;
}

Allocations allocations_create() {
  Allocations as = {.stack_size = 0, .size = 0, .capacity = 0, .buffer = 0};
  return as;
}

void allocations_destroy(Allocations *restrict as) {
  assert(as != NULL);
  as->stack_size = 0;
  as->size       = 0;
  as->capacity   = 0;
  free(as->buffer);
  as->buffer = NULL;
}

static bool allocations_full(Allocations *restrict as) {
  return (as->size + 1) >= as->capacity;
}

static void allocations_grow(Allocations *restrict as) {
  Growth g     = array_growth_u16(as->capacity, sizeof(Allocation));
  as->buffer   = reallocate(as->buffer, g.alloc_size);
  as->capacity = (u16)g.new_capacity;
}

static void allocations_append(Allocations *restrict as, Allocation a) {
  if (allocations_full(as)) {
    allocations_grow(as);
  }

  as->buffer[as->size] = a;
  as->size += 1;
}

void allocations_allocate(Allocations *restrict as, u16 local, Register r) {
  assert(as != NULL);
  allocations_append(as, register_(local, r));
}

static Allocation *existing_allocation(Allocations *restrict as, u16 local) {
  for (u16 i = 0; i < as->size; ++i) {
    Allocation *a = as->buffer + i;
    if (a->local == local) {
      return a;
    }
  }
  return NULL;
}

// #note we want to spill the allocation to the stack,
// so we want to allocate space on the stack for the
// allocation, since it's a local that fits in a register
// we add space for a word of data. #TODO this is hardcoded
// for now, but this number is actually dependent on the
// target machine. and it's dependent on the size of the
// type being allocated. For now, this is good enough.
static void bump_stack_size(Allocations *restrict as) { as->stack_size += 8; }

void allocations_spill(Allocations *restrict as, u16 local) {
  Allocation *a = existing_allocation(as, local);

  if (a != NULL) {
    *a = stack(local, as->stack_size);
  } else {
    allocations_append(as, stack(local, as->stack_size));
  }

  bump_stack_size(as);
}