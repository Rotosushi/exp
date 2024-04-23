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
#ifndef EXP_UTILITY_ARRAY_GROWTH_H
#define EXP_UTILITY_ARRAY_GROWTH_H
#include "utility/int_types.h"

/**
 * @brief The growth factor of array_growth
 *
 * @note With a growth factor of 2, there is
 * amortized constant time append to arrays.
 *
 */
#define ARRAY_GROWTH_FACTOR 2ul

/**
 * @brief The minimum capacity of any array grown
 * with array_growth
 *
 */
#define ARRAY_MIN_CAPACITY 8ul

/**
 * @brief the new capacity of the array, and it's allocation size
 *
 */
typedef struct Growth {
  u64 new_capacity;
  u64 alloc_size;
} Growth;

/**
 * @brief grow current_capacity by ARRAY_GROWTH_FACTOR
 *
 * @param current_capacity
 * @param element_size
 * @return Growth
 */
Growth array_growth(u64 current_capacity, u64 element_size);

#endif // !EXP_UTILITY_ARRAY_GROWTH_H