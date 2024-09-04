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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
/**
 * @brief provides functions for computing the
 * min or max of two values of integer and unsigned
 * integer types.
 *
 */
#ifndef EXP_UTILITY_MINMAX_H
#define EXP_UTILITY_MINMAX_H
#include "utility/int_types.h"

i32 imin(i32 x, i32 y);
i64 lmin(i64 x, i64 y);

u32 umin(u32 x, u32 y);
u64 ulmin(u64 x, u64 y);

i32 imax(i32 x, i32 y);
i64 lmax(i64 x, i64 y);

u32 umax(u32 x, u32 y);
u64 ulmax(u64 x, u64 y);

#endif // !EXP_UTILITY_MINMAX_H
