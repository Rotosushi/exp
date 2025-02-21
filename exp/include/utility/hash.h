// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file utility/hash.h
 */

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
