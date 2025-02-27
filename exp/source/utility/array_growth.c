/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "utility/array_growth.h"
#include "utility/minmax.h"
#include "utility/panic.h"

/**
 * @brief The growth factor of array_growth
 *
 * @note With a growth factor of 2, there is
 * amortized constant time append to arrays.
 *
 */
#define ARRAY_GROWTH_FACTOR 2ul

/**
 * @brief The minimum capacity of any array grown
 * with array_growth
 *
 */
#define ARRAY_MIN_CAPACITY 1ul

Growth64 array_growth_u64(u64 current_capacity, u64 element_size) {
    if (current_capacity == u64_MAX) {
        PANIC("cannot allocate more than u64_MAX elements");
    }

    Growth64 g;
    if (__builtin_mul_overflow(max_u64(current_capacity, ARRAY_MIN_CAPACITY),
                               ARRAY_GROWTH_FACTOR, &g.new_capacity)) {
        g.new_capacity = u64_MAX;
    }

    if (__builtin_mul_overflow(g.new_capacity, element_size, &g.alloc_size)) {
        PANIC("cannot allocate more than u64_MAX");
    }
    return g;
}

Growth32 array_growth_u32(u32 current_capacity, u64 element_size) {
    if (current_capacity == u32_MAX) {
        PANIC("cannot allocate more than u32_MAX elements");
    }

    Growth32 g;
    if (__builtin_mul_overflow(max_u32(current_capacity, ARRAY_MIN_CAPACITY),
                               ARRAY_GROWTH_FACTOR, &g.new_capacity)) {
        g.new_capacity = u32_MAX;
    }

    if (__builtin_mul_overflow(g.new_capacity, element_size, &g.alloc_size)) {
        PANIC("cannot allocate more than u64_MAX");
    }
    return g;
}

Growth16 array_growth_u16(u16 current_capacity, u64 element_size) {
    if (current_capacity == u16_MAX) {
        PANIC("cannot allocate more than u16_MAX elements");
    }

    Growth16 g;
    if (__builtin_mul_overflow(max_u16(current_capacity, ARRAY_MIN_CAPACITY),
                               ARRAY_GROWTH_FACTOR, &g.new_capacity)) {
        g.new_capacity = u16_MAX;
    }

    if (__builtin_mul_overflow(g.new_capacity, element_size, &g.alloc_size)) {
        PANIC("cannot allocate more than u64_MAX");
    }
    return g;
}

Growth8 array_growth_u8(u8 current_capacity, u64 element_size) {
    if (current_capacity == u8_MAX) {
        PANIC("cannot allocate more than u8_MAX elements");
    }

    Growth8 g;
    if (__builtin_mul_overflow(max_u8(current_capacity, ARRAY_MIN_CAPACITY),
                               ARRAY_GROWTH_FACTOR, &g.new_capacity)) {
        g.new_capacity = u8_MAX;
    }

    if (__builtin_mul_overflow(g.new_capacity, element_size, &g.alloc_size)) {
        PANIC("cannot allocate more than u64_MAX");
    }
    return g;
}
