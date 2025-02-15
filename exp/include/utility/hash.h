// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_HASH_H
#define EXP_UTILITY_HASH_H
#include "utility/int_types.h"

/**
 * @brief computes the hash of the given string.
 *
 * @note the algorithm is non-crypotgraphic, and based on djb2 hash
 *
 *
 * @param string
 * @return u64
 */
u64 hash_cstring(char const *restrict string, u64 length);

#endif // !EXP_UTILITY_HASH_H
