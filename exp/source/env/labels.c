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
#include <assert.h>

#include "env/labels.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/panic.h"

void labels_initialize(Labels *labels) {
    assert(labels != nullptr);
    labels->count    = 0;
    labels->capacity = 0;
    labels->buffer   = nullptr;
}

void labels_terminate(Labels *labels) {
    assert(labels != nullptr);
    deallocate(labels->buffer);
    labels->buffer   = nullptr;
    labels->count    = 0;
    labels->capacity = 0;
}

static bool global_labels_full(Labels *labels) {
    assert(labels != nullptr);
    return (labels->count + 1) >= labels->capacity;
}

static void global_labels_grow(Labels *labels) {
    assert(labels != nullptr);
    Growth32 g       = array_growth_u32(labels->capacity, sizeof(StringView));
    labels->buffer   = reallocate(labels->buffer, g.alloc_size);
    labels->capacity = g.new_capacity;
}

u32 labels_insert(Labels *labels, StringView label) {
    assert(labels != nullptr);

    if (global_labels_full(labels)) { global_labels_grow(labels); }

    for (u32 index = 0; index < labels->count; ++index) {
        StringView view = labels->buffer[index];
        if (string_view_equality(view, label)) { return index; }
    }

    u32 index             = labels->count;
    labels->buffer[index] = label;
    labels->count += 1;
    return index;
}

StringView labels_at(Labels *symbols, u32 index) {
    assert(symbols != nullptr);
    assert(index < symbols->count);
    return symbols->buffer[index];
}
