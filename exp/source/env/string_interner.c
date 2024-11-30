/**
 * Copyright (C) 2024 Cade Weinberg
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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "env/string_interner.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/hash.h"

static void string_view_initialize(StringView *target, StringView source) {
    assert(target != nullptr);
    target->length = source.length;
    target->ptr    = callocate(source.length + 1, sizeof(char));
    memcpy((char *)target->ptr, source.ptr, source.length);
}

static void string_view_terminate(StringView *target) {
    assert(target != nullptr);
    target->length = 0;
    deallocate((char *)target->ptr);
    target->ptr = nullptr;
}

#define STRING_INTERNER_MAX_LOAD 0.75

void string_interner_initialize(StringInterner *string_interner) {
    assert(string_interner != nullptr);
    string_interner->count    = 0;
    string_interner->capacity = 0;
    string_interner->buffer   = nullptr;
}

void string_interner_terminate(StringInterner *string_interner) {
    assert(string_interner != nullptr);

    if (string_interner->buffer == nullptr) {
        string_interner->capacity = 0;
        string_interner->count    = 0;
        return;
    }

    for (u64 i = 0; i < string_interner->capacity; ++i) {
        StringView *element = string_interner->buffer + i;
        if (!string_view_empty(*element)) { string_view_terminate(element); }
    }

    string_interner->capacity = 0;
    string_interner->count    = 0;
    deallocate(string_interner->buffer);
    string_interner->buffer = nullptr;
}

static StringView *
string_interner_find(StringView *buffer, u64 capacity, StringView view) {
    u64 index = hash_cstring(view.ptr, view.length) % capacity;
    while (1) {
        StringView *element = buffer + index;
        if ((string_view_empty(*element)) ||
            (string_view_equality(*element, view))) {
            return element;
        }

        index = (index + 1) % capacity;
    }
}

static void string_interner_grow(StringInterner *string_interner) {
    Growth64 g =
        array_growth_u64(string_interner->capacity, sizeof(StringView));
    StringView *elements = callocate(g.new_capacity, sizeof(StringView));

    // if the buffer isn't empty, we need to reinsert
    // all existing elements into the new buffer.
    if (string_interner->buffer != nullptr) {
        for (u64 i = 0; i < string_interner->capacity; ++i) {
            StringView *element = string_interner->buffer + i;
            if (string_view_empty(*element)) { continue; }

            StringView *target =
                string_interner_find(elements, g.new_capacity, *element);
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
                                  StringView sv) {
    assert(string_interner != nullptr);
    if (string_interner_full(string_interner)) {
        string_interner_grow(string_interner);
    }

    StringView *element = string_interner_find(
        string_interner->buffer, string_interner->capacity, sv);
    if (!string_view_empty(*element)) { return *element; }

    string_interner->count++;
    string_view_initialize(element, sv);
    return *element;
}
