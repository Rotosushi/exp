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
 * @file utility/memory.c
 */

#include "utility/memory.h"
#include "utility/assert.h"

/*
 *  [target, target + target_length)
 *  [source, source + source_length)
 *
 * two ranges overlap in memory if any element of
 * range 1 is an element of range 2.
 */
static bool overlaping_ranges(void const *target, u64 target_length,
                              void const *source, u64 source_length) {
    EXP_ASSERT(target != nullptr);
    EXP_ASSERT(source != nullptr);
    u8 const *pt = target;
    u8 const *ps = source;
    if (pt == ps) { return true; }
    if (ps < pt) { return (ps + source_length) > pt; }
    if (ps > pt) { return (pt + target_length) > ps; }
    return false;
}

void memory_copy(void *restrict target, u64 target_length,
                 void const *restrict source, u64 source_length) {
    EXP_ASSERT(target != nullptr);
    EXP_ASSERT(source != nullptr);
    EXP_ASSERT(target_length >= source_length);
    EXP_ASSERT(
        !overlaping_ranges(target, target_length, source, source_length));
    u8 *pt       = target;
    u8 const *ps = source;
    for (u64 index = 0; index < source_length; ++index) {
        pt[index] = ps[index];
    }
}

void memory_move(void *target, u64 target_length, void const *source,
                 u64 source_length) {
    EXP_ASSERT(target != nullptr);
    EXP_ASSERT(source != nullptr);
    EXP_ASSERT(target_length >= source_length);
    if (!overlaping_ranges(target, target_length, source, source_length)) {
        memory_copy(target, target_length, source, source_length);
        return;
    }

    u8 buffer[source_length];
    u8 const *ps = source;
    for (u64 index = 0; index < source_length; ++index) {
        buffer[index] = ps[index];
    }

    u8 *pt = target;
    for (u64 index = 0; index < source_length; ++index) {
        pt[index] = buffer[index];
    }
}

i32 memory_compare(void const *target, u64 target_length, void const *source,
                   u64 source_length) {
    EXP_ASSERT(target != nullptr);
    EXP_ASSERT(source != nullptr);
    EXP_ASSERT(target_length >= source_length);
    u8 const *pt = target;
    u8 const *ps = source;
    for (u64 index = 0; index < source_length; ++index) {
        if (pt[index] != ps[index]) { return pt[index] - ps[index]; }
    }
    return 0;
}
