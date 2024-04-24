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
#ifndef EXP_BACKEND_LIVE_INTERVALS_H
#define EXP_BACKEND_LIVE_INTERVALS_H

#include "utility/int_types.h"

/**
 * @brief models the lifetime of a SSA local within a function.
 *
 * @note The beginning of an interval is the bytecode instruction number
 * which assigns a live value to a given SSA local.
 * the end of an interval is the last bytecode instruction number
 * which uses the value of a SSA local.
 *
 */
typedef struct Interval {
  u16 local;
  u16 begin;
  u16 end;
} Interval;

/**
 * @brief manages a set of live intervals within a function
 *
 */
typedef struct LiveIntervals {
  u16 size;
  u16 capacity;
  Interval *buffer;
} LiveIntervals;

LiveIntervals live_intervals_create();
void live_intervals_destroy(LiveIntervals *restrict li);

#endif // !EXP_BACKEND_LIVE_INTERVALS_H