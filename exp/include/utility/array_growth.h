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
Growth array_growth_u64(u64 current_capacity, u64 element_size);

Growth array_growth_u32(u32 current_capacity, u64 element_size);

Growth array_growth_u16(u16 current_capacity, u64 element_size);

Growth array_growth_u8(u8 current_capacity, u64 element_size);

#endif // !EXP_UTILITY_ARRAY_GROWTH_H
