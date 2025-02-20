/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "utility/int_types.h"

bool i64_in_range_i32(i64 value) {
    return (value >= i32_MIN) && (value <= i32_MAX);
}

bool i64_in_range_i16(i64 value) {
    return (value >= i16_MIN) && (value <= i16_MAX);
}

bool i64_in_range_i8(i64 value) {
    return (value >= i8_MIN) && (value <= i8_MAX);
}

bool i64_in_range_u8(i64 value) { return (value >= 0) && (value <= u8_MAX); }
bool i64_in_range_u16(i64 value) { return (value >= 0) && (value <= u16_MAX); }
bool i64_in_range_u32(i64 value) { return (value >= 0) && (value <= u32_MAX); }
bool i64_in_range_u64(i64 value) { return (value >= 0); }

bool u64_in_range_i64(u64 value) { return (value <= i64_MAX); }
bool u64_in_range_i32(u64 value) { return (value <= i32_MAX); }
bool u64_in_range_i16(u64 value) { return (value <= i16_MAX); }
bool u64_in_range_i8(u64 value) { return (value <= i8_MAX); }

bool u64_in_range_u8(u64 value) { return (value <= u8_MAX); }
bool u64_in_range_u16(u64 value) { return (value <= u16_MAX); }
bool u64_in_range_u32(u64 value) { return (value <= u32_MAX); }
