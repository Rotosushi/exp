/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "imr/local.h"
#include "utility/assert.h"

void local_initialize(Local *local) {
    EXP_ASSERT(local != nullptr);
    local->type     = nullptr;
    local->label    = string_view_create();
    local->lifetime = lifetime_create();
}

void local_update_type(Local *local, Type const *type) {
    EXP_ASSERT(local != nullptr);
    local->type = type;
}

void local_update_label(Local *local, StringView label) {
    EXP_ASSERT(local != nullptr);
    local->label = label;
}

void local_update_first_use(Local *local, u32 first_use) {
    EXP_ASSERT(local != nullptr);
    local->lifetime.first_use = first_use;
}

void local_update_last_use(Local *local, u32 last_use) {
    EXP_ASSERT(local != nullptr);
    local->lifetime.last_use = last_use;
}
