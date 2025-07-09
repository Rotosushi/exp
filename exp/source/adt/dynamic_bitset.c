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

#include <string.h>

#include "adt/dynamic_bitset.h"
#include "support/allocation.h"
#include "support/assert.h"
#include "support/panic.h"

void dynamic_bitset_create(DynamicBitset *restrict bitset) {
    exp_assert(bitset != NULL);
    bitset->capacity = 0;
    bitset->buffer   = NULL;
}

void dynamic_bitset_destroy(DynamicBitset *restrict bitset) {
    exp_assert(bitset != NULL);
    deallocate(bitset->buffer);
    dynamic_bitset_create(bitset);
}

static u64 element_of_index(u64 index) { return index / 64; }
static u64 bit_of_element(u64 index) { return index % 64; }

static bool index_in_range(DynamicBitset const *restrict bitset, u64 index) {
    // an index is located in one of the 64 bits of one of the elements in the
    // dynamic array the index of said element (e) is (e = index / 64). that is,
    // the first 64 elements are in index 0, the next 64 are in index 1, and so
    // on. Capacity is the number of available elements. so if capacity is
    // greater than the given index, we have to grow the array
    return element_of_index(index) < bitset->capacity;
}

void dynamic_bitset_resize(DynamicBitset *restrict bitset, u64 capacity) {
    u64 alloc_size;
    if (__builtin_mul_overflow(
            capacity, sizeof(*bitset->buffer), &alloc_size)) {
        PANIC("dynamic array capacity overflow");
    }

    bitset->buffer = reallocate(bitset->buffer, alloc_size);
    memset(bitset->buffer + bitset->capacity,
           0,
           (capacity - bitset->capacity) * sizeof(*bitset->buffer));
    bitset->capacity = capacity;
}

void dynamic_bitset_set(DynamicBitset *restrict bitset, u64 index) {
    exp_assert(bitset != NULL);
    exp_assert(index_in_range(bitset, index));
    bitset->buffer[element_of_index(index)] |= (1ULL << bit_of_element(index));
}

void dynamic_bitset_clear(DynamicBitset *restrict bitset, u64 index) {
    exp_assert(bitset != NULL);
    exp_assert(index_in_range(bitset, index));
    bitset->buffer[element_of_index(index)] &= ~(1ULL << bit_of_element(index));
}

bool dynamic_bitset_check(DynamicBitset *restrict bitset, u64 index) {
    exp_assert(bitset != NULL);
    exp_assert(index_in_range(bitset, index));
    return (bitset->buffer[element_of_index(index)] >> bit_of_element(index)) &
           1ULL;
}
