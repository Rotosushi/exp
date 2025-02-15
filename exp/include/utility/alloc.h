// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_ALLOC_H
#define EXP_UTILITY_ALLOC_H

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

/**
 * @brief wraps a call to free
 *
 * @param ptr
 */
void deallocate(void *ptr);

#endif // !EXP_UTILITY_ALLOC_H
