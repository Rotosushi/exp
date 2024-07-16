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

#include "backend/x64/allocation.h"
#include "utility/alloc.h"

x64_Allocation *x64_allocation_allocate() {
  x64_Allocation *allocation = callocate(1, sizeof(x64_Allocation));
  return allocation;
}

void x64_allocation_deallocate(x64_Allocation *restrict allocation) {
  deallocate(allocation);
}

bool x64_allocation_location_eq(x64_Allocation *restrict allocation,
                                x64_Location location) {
  if (location.kind != allocation->location.kind) { return 0; }

  switch (location.kind) {
  case ALLOC_GPR:   return location.gpr == allocation->location.gpr;
  case ALLOC_STACK: return location.offset == allocation->location.offset;
  default:          unreachable();
  }
}