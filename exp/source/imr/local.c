/**
 * Copyright (C) 2025 cade-weinberg
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

#include "imr/local.h"
#include "support/allocation.h"
#include "support/assert.h"

Local *local_allocate(u32 ssa) {
    Local *local    = callocate(1, sizeof(Local));
    local->ssa      = ssa;
    local->name     = SV("");
    local->type     = NULL;
    local->lifetime = (Lifetime){.start = 0, .end = 0};
    return local;
}

void local_deallocate(Local *restrict local) {
    exp_assert(local != NULL);
    deallocate(local);
}
