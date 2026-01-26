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

/**
 * @file support/arithmetic.h
 * @brief common arithmetic operations not provided by the standard.
 *
 * @note the standard library will be used whenever possible.
 */

#ifndef EXP_SUPPORT_ARITHMETIC_H
#define EXP_SUPPORT_ARITHMETIC_H

#include "support/assert.h"
#include "support/scalar.h"

/**
 * @brief checks that the given i64 value fits within a i8
 *
 * @note if true, it is lossless to cast to an i8.
 *
 * @param i64
 * @return true (value >= i8_MIN) && (value <= i8_MAX)
 * @return false (value < i8_MIN) || (value > i8_MAX)
 */
inline bool i64_in_range_i8(i64 value) {
    return (value >= i8_MIN) && (value <= i8_MAX);
}

/**
 * @brief checks that the given i64 value fits within a i16
 *
 * @note if true, it is lossless to cast to an i16.
 *
 * @param i64
 * @return true (value >= i16_MIN) && (value <= i16_MAX)
 * @return false (value < i16_MIN) || (value > i16_MAX)
 */
inline bool i64_in_range_i16(i64 value) {
    return (value >= i16_MIN) && (value <= i16_MAX);
}

/**
 * @brief checks that the given i64 value fits within a i32
 *
 * @note if true, it is lossless to cast to an i32.
 *
 * @param i64
 * @return true (value >= i32_MIN) && (value <= i32_MAX)
 * @return false (value < i32_MIN) || (value > i32_MAX)
 */
inline bool i64_in_range_i32(i64 value) {
    return (value >= i32_MIN) && (value <= i32_MAX);
}

/**
 * @brief checks that the given i64 value fits within a u8
 *
 * @note if true, it is lossless to cast to an u8.
 *
 * @param i64
 * @return true (value >= 0) && (value <= u8_MAX)
 * @return false (value < 0) || (value > u8_MAX)
 */
inline bool i64_in_range_u8(i64 value) {
    return (value >= 0) && (value <= u8_MAX);
}

/**
 * @brief checks that the given i64 value fits within a u16
 *
 * @note if true, it is lossless to cast to an i16.
 *
 * @param i64
 * @return true (value >= 0) && (value <= u16_MAX)
 * @return false (value < 0) || (value > u16_MAX)
 */
inline bool i64_in_range_u16(i64 value) {
    return (value >= 0) && (value <= u16_MAX);
}

/**
 * @brief checks that the given i64 value fits within a u32
 *
 * @note if true, it is lossless to cast to an u32.
 *
 * @param i64
 * @return true (value >= 0) && (value <= u32_MAX)
 * @return false (value < 0) || (value > u32_MAX)
 */
inline bool i64_in_range_u32(i64 value) {
    return (value >= 0) && (value <= u32_MAX);
}

/**
 * @brief checks that the given i64 value fits within a u64
 *
 * @note if true, it is lossless to cast to an u64.
 *
 * @param i64
 * @return true (value >= 0)
 * @return false (value < 0)
 */
inline bool i64_in_range_u64(i64 value) { return (value >= 0); }

/**
 * @brief checks that the given u64 value fits within a i8
 *
 * @note if true, it is lossless to cast to an i8.
 *
 * @param u64
 * @return true (value <= i8_MAX)
 * @return false (value > i8_MAX)
 */
inline bool u64_in_range_i8(u64 value) { return (value <= i8_MAX); }

/**
 * @brief checks that the given u64 value fits within a i16
 *
 * @note if true, it is lossless to cast to an i16.
 *
 * @param u64
 * @return true (value <= i16_MAX)
 * @return false (value > i16_MAX)
 */
inline bool u64_in_range_i16(u64 value) { return (value <= i16_MAX); }

/**
 * @brief checks that the given u64 value fits within a i32
 *
 * @note if true, it is lossless to cast to an i32.
 *
 * @param u64
 * @return true (value <= i32_MAX)
 * @return false (value > i32_MAX)
 */
inline bool u64_in_range_i32(u64 value) { return (value <= i32_MAX); }

/**
 * @brief checks that the given u64 value fits within a i64
 *
 * @note if true, it is lossless to cast to an i64.
 *
 * @param u64
 * @return true (value <= i64_MAX)
 * @return false (value > i64_MAX)
 */
inline bool u64_in_range_i64(u64 value) { return (value <= i64_MAX); }

/**
 * @brief checks that the given u64 value fits within a u8
 *
 * @note if true, it is lossless to cast to an i8.
 *
 * @param u64
 * @return true (value <= u8_MAX)
 * @return false (value > u8_MAX)
 */
inline bool u64_in_range_u8(u64 value) { return (value <= u8_MAX); }

/**
 * @brief checks that the given u64 value fits within a u16
 *
 * @note if true, it is lossless to cast to an u16.
 *
 * @param u64
 * @return true (value <= u16_MAX)
 * @return false (value > u16_MAX)
 */
inline bool u64_in_range_u16(u64 value) { return (value <= u16_MAX); }

/**
 * @brief checks that the given u64 value fits within a u32
 *
 * @note if true, it is lossless to cast to an u32.
 *
 * @param u64
 * @return true (value <= u32_MAX)
 * @return false (value > u32_MAX)
 */
inline bool u64_in_range_u32(u64 value) { return (value <= u32_MAX); }

/**
 * @brief returns the minimum of x or y
 *
 * @param i8 x
 * @param i8 y
 * @return i8 (x < y) ? x : y;
 */
inline i8 min_i8(i8 x, i8 y) { return (x < y) ? x : y; }

/**
 * @brief returns the minimum of x or y
 *
 * @param i16 x
 * @param i16 y
 * @return i16 (x < y) ? x : y;
 */
inline i16 min_i16(i16 x, i16 y) { return (x < y) ? x : y; }

/**
 * @brief returns the minimum of x or y
 *
 * @param i32 x
 * @param i32 y
 * @return i32 (x < y) ? x : y;
 */
inline i32 min_i32(i32 x, i32 y) { return (x < y) ? x : y; }

/**
 * @brief returns the minimum of x or y
 *
 * @param i64 x
 * @param i64 y
 * @return i64 (x < y) ? x : y;
 */
inline i64 min_i64(i64 x, i64 y) { return (x < y) ? x : y; }

/**
 * @brief returns the minimum of x or y
 *
 * @param u8 x
 * @param u8 y
 * @return u8 (x < y) ? x : y;
 */
inline u8 min_u8(u8 x, u8 y) { return (x < y) ? x : y; }

/**
 * @brief returns the minimum of x or y
 *
 * @param u16 x
 * @param u16 y
 * @return u16 (x < y) ? x : y;
 */
inline u16 min_u16(u16 x, u16 y) { return (x < y) ? x : y; }

/**
 * @brief returns the minimum of x or y
 *
 * @param u32 x
 * @param u32 y
 * @return u32 (x < y) ? x : y;
 */
inline u32 min_u32(u32 x, u32 y) { return (x < y) ? x : y; }

/**
 * @brief returns the minimum of x or y
 *
 * @param u64 x
 * @param u64 y
 * @return u64 (x < y) ? x : y;
 */
inline u64 min_u64(u64 x, u64 y) { return (x < y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param i8 x
 * @param i8 y
 * @return i8 (x < y) ? x : y;
 */
inline i8 max_i8(i8 x, i8 y) { return (x > y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param i16 x
 * @param i16 y
 * @return i16 (x < y) ? x : y;
 */
inline i16 max_i16(i16 x, i16 y) { return (x > y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param i32 x
 * @param i32 y
 * @return i32 (x < y) ? x : y;
 */
inline i32 max_i32(i32 x, i32 y) { return (x > y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param i64 x
 * @param i64 y
 * @return i64 (x < y) ? x : y;
 */
inline i64 max_i64(i64 x, i64 y) { return (x > y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param u8 x
 * @param u8 y
 * @return u8 (x < y) ? x : y;
 */
inline u8 max_u8(u8 x, u8 y) { return (x > y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param u16 x
 * @param u16 y
 * @return u16 (x < y) ? x : y;
 */
inline u16 max_u16(u16 x, u16 y) { return (x > y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param u32 x
 * @param u32 y
 * @return u32 (x < y) ? x : y;
 */
inline u32 max_u32(u32 x, u32 y) { return (x > y) ? x : y; }

/**
 * @brief returns the maximum of x or y
 *
 * @param u64 x
 * @param u64 y
 * @return u64 (x < y) ? x : y;
 */
inline u64 max_u64(u64 x, u64 y) { return (x > y) ? x : y; }

/**
 * @brief returns the absolute value of the given i8
 *
 * @param i8 value
 * @return i8 |value|
 */
inline i8 abs_i8(i8 value) {
    // #NOTE: when compiling under GCC "-value" undergoes integer promotion
    // to int. Then when we return into a smaller type, this generates a
    // warning. Ideally I would not cast, but it is necessary to suppress the
    // warning.
    return (value < 0) ? (i8)(-value) : value;
}

/**
 * @brief returns the absolute value of the given i8
 *
 * @param i16 value
 * @return i16 |value|
 */
inline i16 abs_i16(i16 value) { return (value < 0) ? (i8)(-value) : value; }

/**
 * @brief returns the absolute value of the given i8
 *
 * @param i32 value
 * @return i32 |value|
 */
inline i32 abs_i32(i32 value) { return (value < 0) ? -value : value; }

/**
 * @brief returns the absolute value of the given i8
 *
 * @param i64 value
 * @return i64 |value|
 */
inline i64 abs_i64(i64 value) { return (value < 0) ? -value : value; }

/**
 * @brief *z = x + y.
 *
 * @param u8 x
 * @param u8 y
 * @param u8 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_u8(u8 x, u8 y, u8 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x + y.
 *
 * @param u16 x
 * @param u16 y
 * @param u16 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_u16(u16 x, u16 y, u16 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x + y.
 *
 * @param u32 x
 * @param u32 y
 * @param u32 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_u32(u32 x, u32 y, u32 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x + y.
 *
 * @param u64 x
 * @param u64 y
 * @param u64 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_u64(u64 x, u64 y, u64 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x + y.
 *
 * @param i8 x
 * @param i8 y
 * @param i8 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_i8(i8 x, i8 y, i8 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x + y.
 *
 * @param i16 x
 * @param i16 y
 * @param i16 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_i16(i16 x, i16 y, i16 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x + y.
 *
 * @param i32 x
 * @param i32 y
 * @param i32 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_i32(i32 x, i32 y, i32 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x + y.
 *
 * @param i64 x
 * @param i64 y
 * @param i64 *z
 * @return true when the addition wraps
 * @return false when the addition does not wrap
 */
inline bool add_i64(i64 x, i64 y, i64 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_add_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param u8 x
 * @param u8 y
 * @param u8 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_u8(u8 x, u8 y, u8 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param u16 x
 * @param u16 y
 * @param u16 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_u16(u16 x, u16 y, u16 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param u32 x
 * @param u32 y
 * @param u32 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_u32(u32 x, u32 y, u32 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param u64 x
 * @param u64 y
 * @param u64 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_u64(u64 x, u64 y, u64 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param i8 x
 * @param i8 y
 * @param i8 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_i8(i8 x, i8 y, i8 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param i16 x
 * @param i16 y
 * @param i16 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_i16(i16 x, i16 y, i16 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param i32 x
 * @param i32 y
 * @param i32 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_i32(i32 x, i32 y, i32 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x - y.
 *
 * @param i64 x
 * @param i64 y
 * @param i64 *z
 * @return true when the subtraction wraps
 * @return false when the subtraction does not wrap
 */
inline bool sub_i64(i64 x, i64 y, i64 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_sub_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param u8 x
 * @param u8 y
 * @param u8 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_u8(u8 x, u8 y, u8 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param u16 x
 * @param u16 y
 * @param u16 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_u16(u16 x, u16 y, u16 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param u32 x
 * @param u32 y
 * @param u32 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_u32(u32 x, u32 y, u32 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param u64 x
 * @param u64 y
 * @param u64 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_u64(u64 x, u64 y, u64 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param i8 x
 * @param i8 y
 * @param i8 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_i8(i8 x, i8 y, i8 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param i16 x
 * @param i16 y
 * @param i16 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_i16(i16 x, i16 y, i16 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param i32 x
 * @param i32 y
 * @param i32 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_i32(i32 x, i32 y, i32 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x * y.
 *
 * @param i64 x
 * @param i64 y
 * @param i64 *z
 * @return true when the multiplication wraps
 * @return false when the multiplication does not wrap
 */
inline bool mul_i64(i64 x, i64 y, i64 *z) {
    EXP_ASSERT(z != NULL);
    return (__builtin_mul_overflow(x, y, z));
}

/**
 * @brief *z = x / y.
 *
 * @param u8 x
 * @param u8 y
 * @param u8 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool div_u8(u8 x, u8 y, u8 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x / y.
 *
 * @param u16 x
 * @param u16 y
 * @param u16 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool div_u16(u16 x, u16 y, u16 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x / y.
 *
 * @param u32 x
 * @param u32 y
 * @param u32 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool div_u32(u32 x, u32 y, u32 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x / y.
 *
 * @param u64 x
 * @param u64 y
 * @param u64 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool div_u64(u64 x, u64 y, u64 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x / y.
 *
 * @param i8 x
 * @param i8 y
 * @param i8 *z
 * @return true when y == 0, or the division would wrap
 * @return false when y != 0, or the division would not wrap
 */
inline bool div_i8(i8 x, i8 y, i8 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if (x == i8_MIN && y == -1) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x / y.
 *
 * @param i16 x
 * @param i16 y
 * @param i16 *z
 * @return true when y == 0, or the division would wrap
 * @return false when y != 0, or the division would not wrap
 */
inline bool div_i16(i16 x, i16 y, i16 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if (x == i8_MIN && y == -1) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x / y.
 *
 * @param i32 x
 * @param i32 y
 * @param i32 *z
 * @return true when y == 0, or the division would wrap
 * @return false when y != 0, or the division would not wrap
 */
inline bool div_i32(i32 x, i32 y, i32 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if (x == i8_MIN && y == -1) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x / y.
 *
 * @param i64 x
 * @param i64 y
 * @param i64 *z
 * @return true when y == 0, or the division would wrap
 * @return false when y != 0, or the division would not wrap
 */
inline bool div_i64(i64 x, i64 y, i64 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if (x == i8_MIN && y == -1) { return true; }
    *z = x / y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param u8 x
 * @param u8 y
 * @param u8 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool mod_u8(u8 x, u8 y, u8 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x % y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param u16 x
 * @param u16 y
 * @param u16 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool mod_u16(u16 x, u16 y, u16 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x % y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param u32 x
 * @param u32 y
 * @param u32 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool mod_u32(u32 x, u32 y, u32 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x % y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param u64 x
 * @param u64 y
 * @param u64 *z
 * @return true when y == 0
 * @return false when y != 0
 */
inline bool mod_u64(u64 x, u64 y, u64 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    *z = x % y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param i8 x
 * @param i8 y
 * @param i8 *z
 * @return true when y == 0, or x == i8_MIN && y == -1
 * @return false when y != 0, or x >= i8_MIN && y != -1
 */
inline bool mod_i8(i8 x, i8 y, i8 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if ((x == i8_MIN) && (y == -1)) { return true; }
    *z = x % y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param i16 x
 * @param i16 y
 * @param i16 *z
 * @return true when y == 0, or x == i16_MIN && y == -1
 * @return false when y != 0, or x >= i16_MIN && y != -1
 */
inline bool mod_i16(i16 x, i16 y, i16 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if ((x == i16_MIN) && (y == -1)) { return true; }
    *z = x % y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param i32 x
 * @param i32 y
 * @param i32 *z
 * @return true when y == 0, or x == i32_MIN && y == -1
 * @return false when y != 0, or x >= i32_MIN && y != -1
 */
inline bool mod_i32(i32 x, i32 y, i32 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if ((x == i32_MIN) && (y == -1)) { return true; }
    *z = x % y;
    return false;
}

/**
 * @brief *z = x % y.
 *
 * @param i64 x
 * @param i64 y
 * @param i64 *z
 * @return true when y == 0, or x == i64_MIN && y == -1
 * @return false when y != 0, or x >= i64_MIN && y != -1
 */
inline bool mod_i64(i64 x, i64 y, i64 *z) {
    EXP_ASSERT(z != NULL);
    if (y == 0) { return true; }
    if ((x == i64_MIN) && (y == -1)) { return true; }
    *z = x % y;
    return false;
}

#endif //  !EXP_SUPPORT_ARITHMETIC_H
