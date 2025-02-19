/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "imr/locals.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

void locals_initialize(Locals *locals) {
    EXP_ASSERT(locals != nullptr);
    locals->length   = 0;
    locals->capacity = 0;
    locals->buffer   = nullptr;
}

void locals_terminate(Locals *locals) {
    EXP_ASSERT(locals != nullptr);
    deallocate(locals->buffer);
    locals_initialize(locals);
}

static bool locals_full(Locals *locals) {
    EXP_ASSERT(locals != nullptr);
    return (locals->length + 1) >= locals->capacity;
}

static void locals_grow(Locals *locals) {
    EXP_ASSERT(locals != nullptr);
    Growth32 g       = array_growth_u32(locals->capacity, sizeof(Local));
    locals->buffer   = reallocate(locals->buffer, g.alloc_size);
    locals->capacity = g.new_capacity;
}

void locals_push(Locals *locals, Local local) {
    EXP_ASSERT(locals != nullptr);
    if (locals_full(locals)) { locals_grow(locals); }
    locals->buffer[locals->length++] = local;
}

void locals_pop_n(Locals *locals, u32 n) {
    EXP_ASSERT(locals != nullptr);
    EXP_ASSERT(locals->length >= n);
    locals->length -= n;
}

Local *locals_lookup(Locals *locals, StringView label);
