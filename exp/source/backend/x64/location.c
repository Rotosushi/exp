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
#include <stddef.h>

#include "backend/x64/location.h"

x64_Location x64_location_gpr(x64_GPR gpr) {
  x64_Location a = {.kind = ALLOC_GPR, .gpr = gpr};
  return a;
}

x64_Location x64_location_stack(i16 offset) {
  x64_Location a = {.kind = ALLOC_STACK, .offset = offset};
  return a;
}

bool x64_location_eq(x64_Location A, x64_Location B) {
  if (A.kind != B.kind) { return 0; }

  switch (A.kind) {
  case ALLOC_GPR:   return A.gpr == B.gpr;
  case ALLOC_STACK: return A.offset == B.offset;
  default:          unreachable();
  }
}