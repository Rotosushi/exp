/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
