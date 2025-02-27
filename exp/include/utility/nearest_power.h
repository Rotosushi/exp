// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_NEAREST_POWER_H
#define EXP_UTILITY_NEAREST_POWER_H

#include "utility/int_types.h"

/**
 * @brief return the nearest power of 2 that
 * is greater than or equal to <value>
 *
 * @param value
 * @return u64
 */
u64 nearest_power_of_two(u64 value);

#endif // !EXP_UTILITY_NEAREST_POWER_H
