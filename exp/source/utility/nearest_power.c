/**
 * Copyright (C) 2025 Cade Weinberg
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

/**
 * @file utility/nearest_power.c
 */

#include <stdint.h>

#include "utility/nearest_power.h"

u64 nearest_power_of_two(u64 value) {
    u64 accumulator = 8, scale_factor = 2;
    while (1) {
        if (accumulator >= value) { return accumulator; }
        // if scaling by an additional power of two would overflow
        // a u64, clamp the result at SIZE_MAX - 1.
        if (__builtin_mul_overflow(accumulator, scale_factor, &accumulator)) {
            return SIZE_MAX - 1;
        }
    }
}
