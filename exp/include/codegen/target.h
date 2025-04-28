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
 * @brief Defines the set of function pointers required of a
 * codegen target
 *
 * @note This is a work in progress, and is not intended
 * as the working version.
 */

#ifndef EXP_CODEGEN_TARGET_H
#define EXP_CODEGEN_TARGET_H

#include "env/context.h"

typedef u64 (*align_of_fn)(Type const *restrict type);
typedef u64 (*size_of_fn)(Type const *restrict type);
// note that this exact type signature will not work with
// the way we have codegen defined as is. Because we use a
// target specific context to perform codegen. This is not
// extensible, which will need to be changed for this to
// possible. Though I believe this is a good direction to
// be working in. We just need to allow the generic context
// to be extended with target specific structures, without
// knowing what they are apriori.
typedef void (*codegen_fn)(Symbol const *restrict symbol,
                           Context *restrict context);
typedef void (*emit_fn)(Symbol const *restrict symbol,
                        Context *restrict context);

#endif // !EXP_CODEGEN_TARGET_H
