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
 * @file assert.h
 * @brief assertion macros
 */

#ifndef EXP_SUPPORT_ASSERT_H
#define EXP_SUPPORT_ASSERT_H

#include <stddef.h>

#include "support/panic.h"

/**
 * @def ASSERT(cond)
 * @brief Asserts that the condition is true.
 *
 * @note if EXP_REMOVE_ASSERTS is defined, this macro will expand to nothing.
 * This assert will occur regardless of if NDEBUG is defined.
 *
 */

#if defined EXP_REMOVE_ASSERTS
#define exp_assert(cond) ((void)0)
#else
#define exp_assert(cond) ((cond) || (PANIC("assertion failed: " #cond), 0))
#endif // !NDEBUG

/**
 * @def ASSERT_DEBUG(cond)
 * @brief Asserts that the condition is true
 *
 * @note if NDEBUG is defined, this macro will expand to nothing.
 * meaning the assertion will not be checked in release builds.
 */

#if defined NDEBUG || defined EXP_REMOVE_ASSERTS
#define exp_assert_debug(cond) ((void)0)
#else
#define exp_assert_debug(cond) exp_assert(cond)
#endif // !NDEBUG

/**
 * @def ASSERT_ALWAYS(cond)
 * @brief Asserts that the condition is true.
 *
 * @note this macro will always be checked, regardless of NDEBUG or
 * EXP_REMOVE_ASSERTS.
 */

#define exp_assert_always(cond)                                                \
    ((cond) || (PANIC("assertion failed: " #cond), 0))

#endif // !EXP_SUPPORT_ASSERT_H
