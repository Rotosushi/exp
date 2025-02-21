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
 * @file utility/break.h
 */

#ifndef EXP_UTILITY_BREAK_H
#define EXP_UTILITY_BREAK_H

#ifndef NDEBUG
    #if defined(__GNUC__) || defined(__clang__)
        #define EXP_BREAK() __builtin_trap()
    #else
        #include <stdlib.h>
        #define EXP_BREAK() abort()
    #endif // if defined(__GNUC__) || defined(__clang__)
#else
    #define EXP_BREAK()
#endif // if EXP_DEBUG

#endif // !EXP_UTILITY_BREAK_H
