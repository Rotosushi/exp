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

#include "support/arithmetic.h"

extern bool i64_in_range_i8(i64 value);
extern bool i64_in_range_i16(i64 value);
extern bool i64_in_range_i32(i64 value);
extern bool i64_in_range_u8(i64 value);
extern bool i64_in_range_u16(i64 value);
extern bool i64_in_range_u32(i64 value);
extern bool i64_in_range_u64(i64 value);
extern bool u64_in_range_i8(u64 value);
extern bool u64_in_range_i16(u64 value);
extern bool u64_in_range_i32(u64 value);
extern bool u64_in_range_i64(u64 value);
extern bool u64_in_range_u8(u64 value);
extern bool u64_in_range_u16(u64 value);
extern bool u64_in_range_u32(u64 value);
extern i8   min_i8(i8 x, i8 y);
extern i16  min_i16(i16 x, i16 y);
extern i32  min_i32(i32 x, i32 y);
extern i64  min_i64(i64 x, i64 y);
extern u8   min_u8(u8 x, u8 y);
extern u16  min_u16(u16 x, u16 y);
extern u32  min_u32(u32 x, u32 y);
extern u64  min_u64(u64 x, u64 y);
extern i8   max_i8(i8 x, i8 y);
extern i16  max_i16(i16 x, i16 y);
extern i32  max_i32(i32 x, i32 y);
extern i64  max_i64(i64 x, i64 y);
extern u8   max_u8(u8 x, u8 y);
extern u16  max_u16(u16 x, u16 y);
extern u32  max_u32(u32 x, u32 y);
extern u64  max_u64(u64 x, u64 y);
extern i8   abs_i8(i8 value);
extern i16  abs_i16(i16 value);
extern i32  abs_i32(i32 value);
extern i64  abs_i64(i64 value);
extern bool add_u8(u8 x, u8 y, u8 *z);
extern bool add_u16(u16 x, u16 y, u16 *z);
extern bool add_u32(u32 x, u32 y, u32 *z);
extern bool add_u64(u64 x, u64 y, u64 *z);
extern bool add_i8(i8 x, i8 y, i8 *z);
extern bool add_i16(i16 x, i16 y, i16 *z);
extern bool add_i32(i32 x, i32 y, i32 *z);
extern bool add_i64(i64 x, i64 y, i64 *z);
extern bool sub_u8(u8 x, u8 y, u8 *z);
extern bool sub_u16(u16 x, u16 y, u16 *z);
extern bool sub_u32(u32 x, u32 y, u32 *z);
extern bool sub_u64(u64 x, u64 y, u64 *z);
extern bool sub_i8(i8 x, i8 y, i8 *z);
extern bool sub_i16(i16 x, i16 y, i16 *z);
extern bool sub_i32(i32 x, i32 y, i32 *z);
extern bool sub_i64(i64 x, i64 y, i64 *z);
extern bool mul_u8(u8 x, u8 y, u8 *z);
extern bool mul_u16(u16 x, u16 y, u16 *z);
extern bool mul_u32(u32 x, u32 y, u32 *z);
extern bool mul_u64(u64 x, u64 y, u64 *z);
extern bool mul_i8(i8 x, i8 y, i8 *z);
extern bool mul_i16(i16 x, i16 y, i16 *z);
extern bool mul_i32(i32 x, i32 y, i32 *z);
extern bool mul_i64(i64 x, i64 y, i64 *z);
extern bool div_u8(u8 x, u8 y, u8 *z);
extern bool div_u16(u16 x, u16 y, u16 *z);
extern bool div_u32(u32 x, u32 y, u32 *z);
extern bool div_u64(u64 x, u64 y, u64 *z);
extern bool div_i8(i8 x, i8 y, i8 *z);
extern bool div_i16(i16 x, i16 y, i16 *z);
extern bool div_i32(i32 x, i32 y, i32 *z);
extern bool div_i64(i64 x, i64 y, i64 *z);
extern bool mod_u8(u8 x, u8 y, u8 *z);
extern bool mod_u16(u16 x, u16 y, u16 *z);
extern bool mod_u32(u32 x, u32 y, u32 *z);
extern bool mod_u64(u64 x, u64 y, u64 *z);
extern bool mod_i8(i8 x, i8 y, i8 *z);
extern bool mod_i16(i16 x, i16 y, i16 *z);
extern bool mod_i32(i32 x, i32 y, i32 *z);
extern bool mod_i64(i64 x, i64 y, i64 *z);
