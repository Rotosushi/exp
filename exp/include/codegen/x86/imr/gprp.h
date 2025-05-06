// Copyright (C) 2025 Cade Weinberg
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

#ifndef EXP_CODEGEN_X86_IMR_GPRP_H
#define EXP_CODEGEN_X86_IMR_GPRP_H

#include "codegen/x86/imr/registers.h"

typedef struct x86_GPRP {
    u32 active;
} x86_GPRP;

inline x86_GPRP x86_gprp_construct() { return (x86_GPRP){.active = 0}; }

bool x86_gprp_aquire(x86_GPRP *restrict pool, x86_GPR gpr);
bool x86_gprp_release(x86_GPRP *restrict pool, x86_GPR gpr);
bool x86_gprp_next_available(x86_GPRP *restrict pool, u8 *gpr_index);

#endif // EXP_CODEGEN_X86_IMR_GPRP_H
