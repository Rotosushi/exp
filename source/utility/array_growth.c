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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "utility/array_growth.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

Growth array_growth(u64 current_capacity, u64 element_size) {
  Growth g;
  g.new_capacity = nearest_power_of_two(current_capacity + 1);

  if (__builtin_mul_overflow(g.new_capacity, element_size, &g.alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }
  return g;
}