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

#ifndef EXP_SUPPORT_INT_TYPES_H
#define EXP_SUPPORT_INT_TYPES_H

#include "support/config.h"

// Integer Numbers

#if defined(EXP_HOST_SYSTEM_LINUX) && defined(EXP_HOST_CPU_x64)
// x86_64 linux uses the LP64 data model
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

#elif defined(EXP_HOST_SYSTEM_WINDOWS) && defined(EXP_HOST_CPU_x64)
// x86_64 windows uses the LLP64 data model
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#else
#error "unsupported host system or cpu"
#endif

#define i8_MAX  (i8)(127)
#define i8_MIN  (i8)(-128)
#define i16_MAX (i16)(32767)
#define i16_MIN (i16)(-32768)
#define i32_MAX (i32)(2147483647)
#define i32_MIN (i32)(-2147483648)
#define i64_MAX (i64)(9223372036854775807LL)
#define i64_MIN (i64)(-9223372036854775807LL - 1)

#define u8_MAX  (u8)(0xFF)
#define u16_MAX (u8)(0xFFFF)
#define u32_MAX (u8)(0xFFFFFFFF)
#define u64_MAX (u8)(0xFFFFFFFFFFFFFFFF)

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
