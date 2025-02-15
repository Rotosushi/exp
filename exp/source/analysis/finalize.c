/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "analysis/finalize.h"
#include "analysis/infer_types.h"
#include "analysis/lifetimes.h"
#include "analysis/validate.h"
#include "utility/assert.h"

ExpResult finalize_function(Function *function, struct Context *context) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    if (infer_types_of_locals(function, context) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    analyze_lifetimes_of_locals(function, context);
    validate_function(function, context);
    return EXP_SUCCESS;
}
