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
#include "backend/x64_gprp.h"

GPRP gprp_create() {
  GPRP gprp = {.bitset = 0};
  return gprp;
}

void gprp_destroy(GPRP *restrict gprp) { gprp->bitset = 0; }

#define SET_BIT(B, r) ((B) |= (u16)(1 << r))
#define CLR_BIT(B, r) ((B) &= (u16)(~(1 << r)))
#define CHK_BIT(B, r) (((B) >> r) & 1)

void gprp_force(GPRP *restrict gprp, X64GPR r) { SET_BIT(gprp->bitset, r); }

bool gprp_any_available(GPRP *restrict gprp, X64GPR *restrict r) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i)) {
      *r = (X64GPR)i;
      return 1;
    }
  }
  return 0;
}

bool gprp_any_available_other_than(GPRP *restrict gprp, X64GPR *restrict r,
                                   X64GPR avoid) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i) && (i != avoid)) {
      *r = (X64GPR)i;
      return 1;
    }
  }
  return 0;
}

bool gprp_allocate(GPRP *restrict gprp, X64GPR *restrict r) {
  if (gprp_any_available(gprp, r)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

bool gprp_allocate_other_than(GPRP *restrict gprp, X64GPR *restrict r,
                              X64GPR avoid) {
  if (gprp_any_available_other_than(gprp, r, avoid)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

void gprp_release(GPRP *restrict gprp, X64GPR r) { CLR_BIT(gprp->bitset, r); }

// static bool gprp_check(GPRP *restrict gprp, X64GPR gpr) {
//   return CHK_BIT(gprp->bitset, gpr);
// }

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT