/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file analysis/analyze.c
 */
#include "analysis/analyze.h"
#include "analysis/infer_types.h"
#include "analysis/lifetimes.h"
#include "analysis/validate.h"
#include "utility/assert.h"

ExpResult analyze_function(Function *function, struct Context *context) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    if (infer_types_of_locals(function, context) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    analyze_lifetimes_of_locals(function, context);
    validate_function(function, context);
    return EXP_SUCCESS;
}
