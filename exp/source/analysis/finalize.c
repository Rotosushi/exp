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
#include <assert.h>
#include <stdlib.h>

#include "analysis/allocation.h"
#include "analysis/finalize.h"
#include "analysis/lifetimes.h"
#include "analysis/typecheck.h"

i32 finalize_function(FunctionBody *function, struct Context *context) {
    assert(function != nullptr);
    assert(context != nullptr);
    if (typecheck_function(function, context) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    analyze_lifetimes_of_locals(function, context);
    allocate_locals(function, context);
    return EXIT_SUCCESS;
}
