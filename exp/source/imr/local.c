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

/**
 * @file imr/local.c
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
