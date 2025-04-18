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

#include "imr/locals.h"
#include "imr/local.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void locals_create(Locals *restrict locals) {
    exp_assert(locals != NULL);
    locals->count    = 0;
    locals->capacity = 0;
    locals->buffer   = NULL;
}

void locals_destroy(Locals *restrict locals) {
    exp_assert(locals != NULL);
    locals->count    = 0;
    locals->capacity = 0;
    deallocate(locals->buffer);
    locals->buffer = NULL;
}

static bool locals_full(Locals const *restrict locals) {
    return locals->capacity <= (locals->count + 1);
}

static void locals_grow(Locals *restrict locals) {
    Growth_u32 g     = array_growth_u32(locals->capacity, sizeof(Local *));
    locals->buffer   = reallocate(locals->buffer, g.alloc_size);
    locals->capacity = g.new_capacity;
}

Local *locals_declare(Locals *restrict locals) {
    if (locals_full(locals)) { locals_grow(locals); }
    Local **local = locals->buffer + locals->count;
    *local        = (Local *)allocate(sizeof(Local));
    local_init(*local, locals->count);
    locals->count += 1;
    return *local;
}

Local *locals_lookup(Locals *restrict locals, u32 ssa) {
    exp_assert(locals != NULL);
    exp_assert(ssa < locals->count);
    return locals->buffer[ssa];
}

Local *locals_lookup_name(Locals *restrict locals, StringView name) {
    for (u32 i = 0; i < locals->count; ++i) {
        if (string_view_equal(locals->buffer[i]->name, name)) {
            return locals->buffer[i];
        }
    }
    return NULL;
}
