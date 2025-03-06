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
 * @file support/scalar.h
 * @brief typedefs for sized scalar types.
 * plus some common utility functions.
 */

#ifndef EXP_SUPPORT_INT_TYPES_H
#define EXP_SUPPORT_INT_TYPES_H

#include <stdint.h>

// Integer Numbers

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define i8_MAX  INT8_MAX
#define i8_MIN  INT8_MIN
#define i16_MAX INT16_MAX
#define i16_MIN INT16_MIN
#define i32_MAX INT32_MAX
#define i32_MIN INT32_MIN
#define i64_MAX INT64_MAX
#define i64_MIN INT64_MIN

#define u8_MAX  UINT8_MAX
#define u16_MAX UINT16_MAX
#define u32_MAX UINT32_MAX
#define u64_MAX UINT64_MAX

bool i64_in_range_i8(i64 value);
bool i64_in_range_i16(i64 value);
bool i64_in_range_i32(i64 value);

bool i64_in_range_u8(i64 value);
bool i64_in_range_u16(i64 value);
bool i64_in_range_u32(i64 value);
bool i64_in_range_u64(i64 value);

bool u64_in_range_i8(u64 value);
bool u64_in_range_i16(u64 value);
bool u64_in_range_i32(u64 value);
bool u64_in_range_i64(u64 value);

bool u64_in_range_u8(u64 value);
bool u64_in_range_u16(u64 value);
bool u64_in_range_u32(u64 value);

i8 min_i8(i8 x, i8 y);
i16 min_i16(i16 x, i16 y);
i32 min_i32(i32 x, i32 y);
i64 min_i64(i64 x, i64 y);

u8 min_u8(u8 x, u8 y);
u16 min_u16(u16 x, u16 y);
u32 min_u32(u32 x, u32 y);
u64 min_u64(u64 x, u64 y);

i8 max_i8(i8 x, i8 y);
i16 max_i16(i16 x, i16 y);
i32 max_i32(i32 x, i32 y);
i64 max_i64(i64 x, i64 y);

u8 max_u8(u8 x, u8 y);
u16 max_u16(u16 x, u16 y);
u32 max_u32(u32 x, u32 y);
u64 max_u64(u64 x, u64 y);

i8 abs_i8(i8 value);
i16 abs_i16(i16 value);
i32 abs_i32(i32 value);
i64 abs_i64(i64 value);

// Real Numbers

typedef float f32;
typedef double f64;

#endif // !EXP_SUPPORT_INT_TYPES_H
