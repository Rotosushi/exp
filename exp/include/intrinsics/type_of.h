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
 * @file intrinsics/type_of.h
 */

#ifndef EXP_INTRINSICS_TYPE_OF_H
#define EXP_INTRINSICS_TYPE_OF_H

#include "env/context.h"
#include "imr/function.h"
#include "imr/type.h"
#include "imr/value.h"

Type const *type_of_scalar(Scalar scalar, Context *context);
Type const *type_of_operand(Operand operand, Context *context);
Type const *type_of_value(Value *value, Context *context);
Type const *type_of_function(Function *body, Context *context);

#endif // !EXP_INTRINSICS_TYPE_OF_H
