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
#ifndef EXP_INTRINSICS_TYPE_OF_H
#define EXP_INTRINSICS_TYPE_OF_H

#include "env/context.h"
#include "imr/type.h"
#include "imr/value.h"

/**
 * @brief return the type of the given value
 *
 * @param value
 * @param context
 * @return Type*
 */
Type const *type_of_value(Value const *restrict value,
                          Context *restrict context);

/**
 * @brief return the type of the given function
 *
 * @note This is computed using the type annotations of the arguments and
 * the return type of the function. It does not take into account the
 * actual implementation of the function. The "typecheck" pass computes
 * the type of the function from the implementation, and checks that it
 * matches the type annotations, if there are explicit type annotation,
 * otherwise it fills in the type annotation. (the only optional type
 * annotation is the return type.)
 *
 * @param body
 * @param context
 * @return Type*
 */
Type const *type_of_function(Function *restrict body,
                             Context *restrict context);

Type const *type_of_operand(Operand operand, Context *restrict context);

#endif // !EXP_INTRINSICS_TYPE_OF_H
