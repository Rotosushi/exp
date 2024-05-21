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

X64GPRP x64gprp_create() {
  X64GPRP gprp = {.bitset = 0};
  return gprp;
}

void x64gprp_destroy(X64GPRP *restrict gprp) { gprp->bitset = 0; }

#define SET_BIT(B, r) ((B) |= (u16)(1 << r))
#define CLR_BIT(B, r) ((B) &= (u16)(~(1 << r)))
#define CHK_BIT(B, r) (((B) >> r) & 1)

void x64gprp_aquire(X64GPRP *restrict gprp, X64GPR r) {
  SET_BIT(gprp->bitset, r);
}

bool x64gprp_any_available(X64GPRP *restrict gprp, X64GPR *restrict r) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i)) {
      *r = (X64GPR)i;
      return 1;
    }
  }
  return 0;
}

bool x64gprp_any_available_other_than(X64GPRP *restrict gprp,
                                      X64GPR *restrict r,
                                      X64GPR avoid) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i) && (i != avoid)) {
      *r = (X64GPR)i;
      return 1;
    }
  }
  return 0;
}

bool x64gprp_allocate(X64GPRP *restrict gprp, X64GPR *restrict r) {
  if (x64gprp_any_available(gprp, r)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

bool x64gprp_allocate_other_than(X64GPRP *restrict gprp,
                                 X64GPR *restrict r,
                                 X64GPR avoid) {
  if (x64gprp_any_available_other_than(gprp, r, avoid)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

void x64gprp_release(X64GPRP *restrict gprp, X64GPR r) {
  CLR_BIT(gprp->bitset, r);
}

// static bool gprp_check(X64GPRP *restrict gprp, X64GPR gpr) {
//   return CHK_BIT(gprp->bitset, gpr);
// }

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT