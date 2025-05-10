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

#ifndef EXP_ANALYSIS_VALIDATE_H
#define EXP_ANALYSIS_VALIDATE_H

#include "env/context.h"

/**
 * @brief validates that the given expression does not break any
 * of the enforced invariants. This is for internal consistency,
 * An error here is an error in the compiler, not user code.
 *
 * @note:
 * - each local is declared at most once
 * - each local has a known type.
 * - each use of a local occurs after it's declaration
 * - each lifetime of a local is bound within the function
 * - each use of a local is typesafe
 * - each use of a value is typesafe
 * - each use of a label is typesafe
 *
 */
bool validate(Function const *restrict expression, Context *restrict context);

#endif // !EXP_ANALYSIS_VALIDATE_H
