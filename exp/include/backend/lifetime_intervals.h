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
 * the last_use of an interval is the last bytecode instruction number
 * which uses the value of a SSA local.
 *
 */
typedef struct Interval {
  u16 local;
  u16 first_use;
  u16 last_use;
} Interval;

/**
 * @brief manages a set of lifetime intervals within a function
 *
 */
typedef struct LifetimeIntervals {
  u16 size;
  u16 capacity;
  Interval *buffer;
} LifetimeIntervals;

LifetimeIntervals lifetime_intervals_create();
void lifetime_intervals_destroy(LifetimeIntervals *restrict li);

/**
 * @brief insert a new interval, sorted in order of increasing first_use.
 *
 * @param li
 * @param i
 */
void lifetime_intervals_insert_sorted(LifetimeIntervals *restrict li,
                                      Interval i);

#endif // !EXP_BACKEND_LIVE_INTERVALS_H