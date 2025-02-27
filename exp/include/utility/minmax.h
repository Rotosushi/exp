// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

/**
 * @brief provides functions for computing the
 * min or max of two values of integer and unsigned
 * integer types.
 *
 */
#ifndef EXP_UTILITY_MINMAX_H
#define EXP_UTILITY_MINMAX_H
#include "utility/int_types.h"

i32 min_i32(i32 x, i32 y);
i64 min_i64(i64 x, i64 y);

u32 min_u32(u32 x, u32 y);
u64 min_u64(u64 x, u64 y);

i32 max_i32(i32 x, i32 y);
i64 max_i64(i64 x, i64 y);

u8 max_u8(u8 x, u8 y);
u16 max_u16(u16 x, u16 y);
u32 max_u32(u32 x, u32 y);
u64 max_u64(u64 x, u64 y);

#endif // !EXP_UTILITY_MINMAX_H
