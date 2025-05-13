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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_INTRINSICS_ALIGNOF_H
#define EXP_INTRINSICS_ALIGNOF_H
#include <stddef.h>

#include "env/context.h"
#include "imr/type.h"

/**
 * @brief returns the x86-64 alignment of the <type>
 *
 * @note this is the number of bytes to align a <value> with <type> to.
 *
 * @todo Take into account the target architecture. When we support more than
 * x86-64, my guess is we need a target triple kind of deal held in the context.
 * And the target needs to provide maybe a function pointer to an
 * implementation, and we can store a set of these pointers in the context, so
 * different targets can override the implementations with their own valid ones.
 *
 * @param type
 * @return u64
 */
u64 x86_align_of(Context *restrict context, Type const *restrict type);

#endif // !EXP_INTRINSICS_ALIGNOF_H
