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
#ifndef EXP_UTILITY_INT_TYPES_H
#define EXP_UTILITY_INT_TYPES_H
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define i8_MAX INT8_MAX
#define i8_MIN INT8_MIN

#define i16_MAX INT16_MAX
#define i16_MIN INT16_MIN

#define i32_MAX INT32_MAX
#define i32_MIN INT32_MIN

#define i64_MAX INT64_MAX
#define i64_MIN INT64_MIN

bool i64_in_range_i16(i64 value);

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define u8_MAX  UINT8_MAX
#define u16_MAX UINT16_MAX
#define u32_MAX UINT32_MAX
#define u64_MAX UINT64_MAX

#endif // !EXP_UTILITY_INT_TYPES_H
