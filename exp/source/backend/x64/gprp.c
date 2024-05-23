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

#include "backend/x64/gprp.h"
#include "utility/alloc.h"

x64_GPRP x64_gprp_create() {
  x64_GPRP gprp = {.bitset = 0,
                   .buffer = callocate(16, sizeof(x64_Allocation *))};
  return gprp;
}

void x64_gprp_destroy(x64_GPRP *restrict gprp) {
  gprp->bitset = 0;
  for (u8 i = 0; i < 16; ++i) {
    deallocate(gprp->buffer[i]);
  }
  deallocate(gprp->buffer);
}

#define SET_BIT(B, r) ((B) |= (u16)(1 << r))
#define CLR_BIT(B, r) ((B) &= (u16)(~(1 << r)))
#define CHK_BIT(B, r) (((B) >> r) & 1)

void x64_gprp_aquire(x64_GPRP *restrict gprp, x64_GPR r) {
  SET_BIT(gprp->bitset, r);
}

void x64_gprp_release(x64_GPRP *restrict gprp, x64_GPR r) {
  CLR_BIT(gprp->bitset, r);
  gprp->buffer[r] = NULL;
}

bool x64_gprp_any_available(x64_GPRP *restrict gprp, x64_GPR *restrict r) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i)) {
      *r = (x64_GPR)i;
      return 1;
    }
  }
  return 0;
}

bool x64_gprp_allocate(x64_GPRP *restrict gprp,
                       x64_Allocation *restrict allocation) {
  x64_GPR gpr;
  if (x64_gprp_any_available(gprp, &gpr)) {
    x64_gprp_allocate_to_gpr(gprp, gpr, allocation);
    return 1;
  }

  return 0;
}

void x64_gprp_allocate_to_gpr(x64_GPRP *restrict gprp,
                              x64_GPR gpr,
                              x64_Allocation *restrict allocation) {
  SET_BIT(gprp->bitset, gpr);
  gprp->buffer[gpr]    = allocation;
  allocation->location = x64_location_reg(gpr);
}

bool x64_gprp_reallocate(x64_GPRP *restrict gprp,
                         x64_Allocation *restrict allocation) {
  assert(allocation->location.kind == ALLOC_GPR);
  x64_GPR gpr;
  if (x64_gprp_any_available(gprp, &gpr)) {
    x64_gprp_release(gprp, allocation->location.gpr);
    x64_gprp_allocate_to_gpr(gprp, gpr, allocation);
    return 1;
  }
  return 0;
}

x64_Allocation *x64_gprp_allocation_at(x64_GPRP *restrict gprp, x64_GPR gpr) {
  return gprp->buffer[gpr];
}

x64_Allocation *x64_gprp_allocation_of(x64_GPRP *restrict gprp, u16 ssa) {
  for (u8 i = 0; i < 16; ++i) {
    x64_Allocation *cursor = gprp->buffer[i];
    if (cursor == NULL) { continue; }
    if (cursor->ssa == ssa) { return cursor; }
  }

  return NULL;
}

x64_Allocation *x64_gprp_oldest_allocation(x64_GPRP *restrict gprp) {
  x64_Allocation *oldest = NULL;

  for (u8 i = 0; i < 16; ++i) {
    x64_Allocation *cursor = gprp->buffer[i];
    if (cursor == NULL) { continue; }

    if ((oldest == NULL) ||
        (oldest->lifetime.last_use < cursor->lifetime.last_use)) {
      oldest = cursor;
    }
  }
  return oldest;
}

void x64_gprp_release_expired_allocations(x64_GPRP *restrict gprp, u16 Idx) {
  for (u8 i = 0; i < 16; ++i) {
    x64_Allocation *cursor = gprp->buffer[i];
    if (cursor == NULL) { continue; }

    if (cursor->lifetime.last_use <= Idx) {
      deallocate(cursor);
      x64_gprp_release(gprp, i);
    }
  }
}

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT