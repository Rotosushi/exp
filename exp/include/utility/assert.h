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
 * @file utility/assert.h
 */

#ifndef EXP_UTILITY_ASSERT_H
#define EXP_UTILITY_ASSERT_H

#include "utility/config.h"

#ifndef EXP_REMOVE_ASSERTS
    #include "utility/break.h"
    #include "utility/panic.h"
    #define EXP_ASSERT(expression) ((expression) || (PANIC(#expression), 1))
#else
    #define EXP_ASSERT(expression)
#endif

#endif // EXP_UTILITY_ASSERT_H
