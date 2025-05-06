/**
 * Copyright (C) 2025 Cade Weinberg
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

#include "codegen/x86/imr/gprp.h"

extern x86_GPRP x86_gprp_construct();

#define SET(pool, index) ((pool->active) |= (u32)(1 << index))
#define CLR(pool, index) ((pool->active) &= (u32)(~(1 << index)))
#define GET(pool, index) ((pool->active >> index) & 1)

bool x86_gprp_aquire(x86_GPRP *restrict pool, x86_GPR gpr) {
    u8 index = x86_gpr_to_index(gpr);
    if (GET(pool, index)) { return false; }

    SET(pool, index);
    return true;
}

bool x86_gprp_release(x86_GPRP *restrict pool, x86_GPR gpr) {
    u8 index = x86_gpr_to_index(gpr);
    if (GET(pool, index)) {
        CLR(pool, index);
        return true;
    }

    return false;
}

bool x86_gprp_next_available(x86_GPRP *restrict pool, u8 *gpr_index) {
    for (u8 index = 0; index < 16; ++index) {
        if (!GET(pool, index)) {
            SET(pool, index);
            *gpr_index = index;
            return true;
        }
    }
    return false;
}

#undef SET
#undef CLR
#undef GET
