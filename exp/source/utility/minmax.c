/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "utility/minmax.h"

i32 min_i32(i32 x, i32 y) { return (x > y) ? y : x; }
i64 min_i64(i64 x, i64 y) { return (x > y) ? y : x; }

u32 min_u32(u32 x, u32 y) { return (x > y) ? y : x; }
u64 min_u64(u64 x, u64 y) { return (x > y) ? y : x; }

i32 max_i32(i32 x, i32 y) { return (x > y) ? x : y; }
i64 max_i64(i64 x, i64 y) { return (x > y) ? x : y; }

u8 max_u8(u8 x, u8 y) { return (x > y) ? x : y; }
u16 max_u16(u16 x, u16 y) { return (x > y) ? x : y; }
u32 max_u32(u32 x, u32 y) { return (x > y) ? x : y; }
u64 max_u64(u64 x, u64 y) { return (x > y) ? x : y; }
