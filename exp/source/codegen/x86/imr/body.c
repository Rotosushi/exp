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

#include "codegen/x86/imr/body.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void x86_body_create(x86_Body *restrict body) {
    exp_assert(body != NULL);
    body->length   = 0;
    body->capacity = 0;
    body->buffer   = NULL;
}

void x86_body_destroy(x86_Body *restrict body) {
    exp_assert(body != NULL);
    for (u32 index = 0; index < body->length; ++index) {
        x86_block_destroy(body->buffer + index);
    }
    deallocate(body->buffer);
    x86_body_create(body);
}

u32 x86_body_length(x86_Body const *restrict body) {
    exp_assert(body != NULL);
    return body->length;
}

static bool x86_body_full(x86_Body const *restrict body) {
    return (body->length + 1) >= body->capacity;
}

static void x86_body_grow(x86_Body *restrict body) {
    Growth_u32 g   = array_growth_u32(body->capacity, sizeof(*body->buffer));
    body->buffer   = reallocate(body->buffer, g.alloc_size);
    body->capacity = g.new_capacity;
}

void x86_body_insert(x86_Body *restrict body, u32 position) {
    exp_assert(body != NULL);
    exp_assert(position <= body->capacity);
    exp_assert(position <= body->length);
    if (x86_body_full(body)) { x86_body_grow(body); }

    for (u32 index = body->length; index < position; --index) {
        body->buffer[index] = body->buffer[index - 1];
    }

    x86_block_create(body->buffer + position);
    body->length += 1;
}

u32 x86_body_append(x86_Body *restrict body) {
    exp_assert(body != NULL);
    u32 position = body->length;
    x86_body_insert(body, position);
    return position;
}

void x86_body_prepend(x86_Body *restrict body) {
    exp_assert(body != NULL);
    x86_body_insert(body, 0);
}

x86_Block *x86_body_at(x86_Body *restrict body, u32 index) {
    exp_assert(body != NULL);
    exp_assert(index < body->length);
    return body->buffer + index;
}

void print_x86_body(String *restrict string, x86_Body const *restrict body) {
    exp_assert(string != NULL);
    exp_assert(body != NULL);
    for (u32 index = 0; index < body->length; ++index) {
        print_x86_block(string, body->buffer + index);
    }
}
