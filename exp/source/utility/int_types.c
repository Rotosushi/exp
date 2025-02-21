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
 * @file utility/int_types.c
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
