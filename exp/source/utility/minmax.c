/**
 * Copyright (C) 2024 Cade Weinberg
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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
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
