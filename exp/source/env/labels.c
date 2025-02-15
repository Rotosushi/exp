/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "env/labels.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

void labels_initialize(Labels *labels) {
    EXP_ASSERT(labels != nullptr);
    labels->count    = 0;
    labels->capacity = 0;
    labels->buffer   = nullptr;
}

void labels_terminate(Labels *labels) {
    EXP_ASSERT(labels != nullptr);
    deallocate(labels->buffer);
    labels->buffer   = nullptr;
    labels->count    = 0;
    labels->capacity = 0;
}

static bool global_labels_full(Labels *labels) {
    EXP_ASSERT(labels != nullptr);
    return (labels->count + 1) >= labels->capacity;
}

static void global_labels_grow(Labels *labels) {
    EXP_ASSERT(labels != nullptr);
    Growth32 g       = array_growth_u32(labels->capacity, sizeof(StringView));
    labels->buffer   = reallocate(labels->buffer, g.alloc_size);
    labels->capacity = g.new_capacity;
}

u32 labels_insert(Labels *labels, StringView label) {
    EXP_ASSERT(labels != nullptr);

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
    EXP_ASSERT(symbols != nullptr);
    EXP_ASSERT(index < symbols->count);
    return symbols->buffer[index];
}
