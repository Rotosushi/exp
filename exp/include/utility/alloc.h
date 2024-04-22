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
#ifndef EXP_UTILITY_ALLOC_H
#define EXP_UTILITY_ALLOC_H
#include <stdlib.h>

#include "utility/int_types.h"

/**
 * @brief wraps a call to malloc, PANIC on failure
 *
 * @param size
 * @return void*
 */
void *allocate(u64 size);

/**
 * @brief wraps a call to calloc, PANIC on failure
 *
 * @param num
 * @param size
 * @return void*
 */
void *callocate(u64 num, u64 size);

/**
 * @brief wraps a call to realloc, PANIC on failure
 *
 * @param ptr
 * @param size
 * @return * void*
 */
void *reallocate(void *ptr, u64 size);

#endif // !EXP_UTILITY_ALLOC_H