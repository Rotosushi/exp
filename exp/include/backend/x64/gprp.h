// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X64_GPRP_H
#define EXP_BACKEND_X64_GPRP_H

#include "backend/x64/allocation.h"
#include "backend/x64/registers.h"

/**
 * @brief General Purpose Register Pool keeps track of which
 * general purpose registers are currently allocated.
 *
 */
typedef struct x64_GPRP {
  u16 bitset;
  x64_Allocation **buffer;
} x64_GPRP;

x64_GPRP x64_gprp_create();
void x64_gprp_destroy(x64_GPRP *restrict gprp);
bool x64_gprp_any_available(x64_GPRP *restrict gprp, x64_GPR *restrict r);
void x64_gprp_aquire(x64_GPRP *restrict gprp, x64_GPR r);
void x64_gprp_release(x64_GPRP *restrict gprp, x64_GPR r);
bool x64_gprp_allocate(x64_GPRP *restrict gprp,
                       x64_Allocation *restrict allocation);
void x64_gprp_allocate_to_gpr(x64_GPRP *restrict gprp,
                              x64_GPR gpr,
                              x64_Allocation *restrict allocation);
bool x64_gprp_reallocate(x64_GPRP *restrict gprp,
                         x64_Allocation *restrict allocation);
x64_Allocation *x64_gprp_allocation_at(x64_GPRP *restrict gprp, x64_GPR gpr);
x64_Allocation *x64_gprp_allocation_of(x64_GPRP *restrict gprp, u16 ssa);
x64_Allocation *x64_gprp_oldest_allocation(x64_GPRP *restrict gprp);

void x64_gprp_release_expired_allocations(x64_GPRP *restrict gprp, u16 Idx);

#endif // !EXP_BACKEND_X64_GPRP_H