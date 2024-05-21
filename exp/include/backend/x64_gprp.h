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
typedef struct X64GPRP {
  u16 bitset;
} X64GPRP;

X64GPRP x64gprp_create();
void x64gprp_destroy(X64GPRP *restrict gprp);
void x64gprp_aquire(X64GPRP *restrict gprp, X64GPR r);
bool x64gprp_any_available(X64GPRP *restrict gprp, X64GPR *restrict r);
bool x64gprp_any_available_other_than(X64GPRP *restrict gprp,
                                      X64GPR *restrict r,
                                      X64GPR avoid);
bool x64gprp_allocate(X64GPRP *restrict gprp, X64GPR *restrict r);
bool x64gprp_allocate_other_than(X64GPRP *restrict gprp,
                                 X64GPR *restrict r,
                                 X64GPR avoid);
void x64gprp_release(X64GPRP *restrict gprp, X64GPR r);

#endif // !EXP_BACKEND_X64_GPRP_H