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
 * @file intrinsics/align_of.h
 */

#ifndef EXP_INTRINSICS_ALIGNOF_H
#define EXP_INTRINSICS_ALIGNOF_H
#include <stddef.h>

#include "imr/type.h"

/**
 * @brief returns the native alignment of the <type>
 *
 * @note this is the number of bytes to align a <value> with <type> to.
 *
 * @param type
 * @return u64
 */
u64 align_of(Type const *type);

#endif // !EXP_INTRINSICS_ALIGNOF_H
