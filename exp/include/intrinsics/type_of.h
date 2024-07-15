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
#include "imr/instruction.h"
#include "imr/type.h"
#include "imr/value.h"

/**
 * @brief return the type of the given value
 *
 * @param value
 * @param context
 * @return Type*
 */
Type *type_of_value(Value *restrict value, Context *restrict context);

Type *type_of_function(FunctionBody *restrict body, Context *restrict context);

Type *type_of_operand(Operand operand, Context *restrict context);

#endif // !EXP_INTRINSICS_TYPE_OF_H