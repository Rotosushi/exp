/**
 * Copyright (C) 2024 Cade Weinberg
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
// #include <assert.h>
// #include <stdlib.h>

#include "analysis/finalize.h"
#include "analysis/allocation.h"
#include "analysis/infer_types.h"
#include "analysis/lifetimes.h"
#include "utility/assert.h"

ExpResult finalize_function(Function *function, struct Context *context) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    if (infer_types_of_locals(function, context) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    analyze_lifetimes_of_locals(function, context);
    allocate_locals(function, context);
    return EXP_SUCCESS;
}
