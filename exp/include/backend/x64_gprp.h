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

#include "backend/x64_gpr.h"

/**
 * @brief General Purpose Register Pool keeps track of which
 * general purpose registers are currently allocated.
 *
 */
typedef struct GPRP {
  u16 bitset;
} GPRP;

GPRP gprp_create();
void gprp_destroy(GPRP *restrict gprp);
void gprp_force(GPRP *restrict gprp, X64GPR r);
bool gprp_any_available(GPRP *restrict gprp, X64GPR *restrict r);
bool gprp_any_available_other_than(GPRP *restrict gprp, X64GPR *restrict r,
                                   X64GPR avoid);
bool gprp_allocate(GPRP *restrict gprp, X64GPR *restrict r);
bool gprp_allocate_other_than(GPRP *restrict gprp, X64GPR *restrict r,
                              X64GPR avoid);
void gprp_release(GPRP *restrict gprp, X64GPR r);

#endif // !EXP_BACKEND_X64_GPRP_H