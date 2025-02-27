/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include <math.h>

#include "env/string_interner.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/hash.h"

#define STRING_INTERNER_MAX_LOAD 0.75

void string_interner_initialize(StringInterner *string_interner) {
    EXP_ASSERT(string_interner != nullptr);
    string_interner->count    = 0;
    string_interner->capacity = 0;
    string_interner->buffer   = nullptr;
}

void string_interner_terminate(StringInterner *string_interner) {
    EXP_ASSERT(string_interner != nullptr);

    if (string_interner->buffer == nullptr) {
        string_interner->capacity = 0;
        string_interner->count    = 0;
        return;
    }

    for (u32 i = 0; i < string_interner->capacity; ++i) {
        ConstantString **element = string_interner->buffer + i;
        if (*element == nullptr) { continue; }
        constant_string_deallocate(*element);
        *element = nullptr;
    }

    string_interner->capacity = 0;
    string_interner->count    = 0;
    deallocate(string_interner->buffer);
    string_interner->buffer = nullptr;
}

static ConstantString **string_interner_find(ConstantString **buffer,
                                             u32 capacity, u64 length,
                                             char const *ptr) {
    u32 index = hash_cstring(ptr, length) % capacity;
    while (1) {
        ConstantString **element = buffer + index;
        if ((*element == nullptr) ||
            (constant_string_equality(*element,
                                      string_view_from_str(ptr, length)))) {
            return element;
        }

        index = (index + 1) % capacity;
    }
}

static void string_interner_grow(StringInterner *string_interner) {
    Growth32 g =
        array_growth_u32(string_interner->capacity, sizeof(ConstantString *));
    ConstantString **elements =
        callocate(g.new_capacity, sizeof(ConstantString *));

    // if the buffer isn't empty, we need to reinsert
    // all existing elements into the new buffer.
    if (string_interner->buffer != nullptr) {
        for (u64 i = 0; i < string_interner->capacity; ++i) {
            ConstantString **element = string_interner->buffer + i;
            if (*element == nullptr) { continue; }

            ConstantString **target =
                string_interner_find(elements, g.new_capacity,
                                     (*element)->length, (*element)->buffer);
            // do a "shallow copy". This is safe because we are in a
            // situation where it is acceptable to "move" the data from
            // the source string to the destination string.
            *target = *element;
        }

        deallocate(string_interner->buffer);
    }

    string_interner->capacity = g.new_capacity;
    string_interner->buffer   = elements;
}

static bool string_interner_full(StringInterner *string_interner) {
    u64 load_limit = (u64)floor((double)string_interner->capacity *
                                STRING_INTERNER_MAX_LOAD);
    return (string_interner->count + 1) >= load_limit;
}

StringView string_interner_insert(StringInterner *string_interner,
                                  StringView view) {
    EXP_ASSERT(string_interner != nullptr);
    if (string_interner_full(string_interner)) {
        string_interner_grow(string_interner);
    }

    ConstantString **element =
        string_interner_find(string_interner->buffer, string_interner->capacity,
                             view.length, view.ptr);
    if (*element != nullptr) { return constant_string_to_view(*element); }

    string_interner->count++;
    *element = constant_string_allocate(view);
    return constant_string_to_view(*element);
}
