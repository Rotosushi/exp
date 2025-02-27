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
#ifndef EXP_ANALYSIS_VALIDATE_H
#define EXP_ANALYSIS_VALIDATE_H

#include "imr/function.h"
#include "utility/result.h"

/**
 * @brief asserts that the function is correct in so far as the IR in
 * concerned.
 *
 * @note asserts the following:
 * 	each local has a valid static type, a valid lifetime, and a valid
 * allocation. each instruction using a local uses it in a typesafe manner. each
 * instruction using a value uses it in a typesafe manner. each instruction
 * using a label uses it in a typesafe manner.
 */
ExpResult validate_function(Function *function, struct Context *context);

#endif // EXP_ANALYSIS_VALIDATE_H
