// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ANALYSIS_FINALIZE_H
#define EXP_ANALYSIS_FINALIZE_H

#include "imr/function.h"
#include "utility/result.h"

/**
 * @brief infers the type, lifetime, and location
 * of each local variable within the function.
 */
ExpResult finalize_function(Function *function, struct Context *context);

#endif // !EXP_ANALYSIS_FINALIZE_H
