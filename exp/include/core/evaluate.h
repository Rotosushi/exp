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
 * @file core/evaluate.h
 */

#ifndef EXP_CORE_EVALUATE_H
#define EXP_CORE_EVALUATE_H

#include "env/context.h"
#include "utility/result.h"

/**
 * @brief Implements the "universal" evaluation function on the given context.
 *
 * @note expects the context to contain a main subroutine.
 * immediately returns EXP_SUCCESS if there is no entry point.
 *
 * @todo add support for evaluating a specific function within a context.
 */
ExpResult evaluate(Context *context);

#endif // !EXP_CORE_EVALUATE_H
