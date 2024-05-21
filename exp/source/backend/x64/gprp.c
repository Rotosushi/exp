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
#include "backend/x64/gprp.h"

x64_GPRP x64_gprp_create() {
  x64_GPRP gprp = {.bitset = 0};
  return gprp;
}

void x64_gprp_destroy(x64_GPRP *restrict gprp) { gprp->bitset = 0; }

#define SET_BIT(B, r) ((B) |= (u16)(1 << r))
#define CLR_BIT(B, r) ((B) &= (u16)(~(1 << r)))
#define CHK_BIT(B, r) (((B) >> r) & 1)

void x64_gprp_aquire(x64_GPRP *restrict gprp, x64_GPR r) {
  SET_BIT(gprp->bitset, r);
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

bool x64_gprp_any_available_other_than(x64_GPRP *restrict gprp,
                                       x64_GPR *restrict r,
                                       x64_GPR avoid) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i) && (i != avoid)) {
      *r = (x64_GPR)i;
      return 1;
    }
  }
  return 0;
}

bool x64_gprp_allocate(x64_GPRP *restrict gprp, x64_GPR *restrict r) {
  if (x64_gprp_any_available(gprp, r)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

bool x64_gprp_allocate_other_than(x64_GPRP *restrict gprp,
                                  x64_GPR *restrict r,
                                  x64_GPR avoid) {
  if (x64_gprp_any_available_other_than(gprp, r, avoid)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

void x64_gprp_release(x64_GPRP *restrict gprp, x64_GPR r) {
  CLR_BIT(gprp->bitset, r);
}

// static bool gprp_check(x64_GPRP *restrict gprp, x64_GPR gpr) {
//   return CHK_BIT(gprp->bitset, gpr);
// }

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT