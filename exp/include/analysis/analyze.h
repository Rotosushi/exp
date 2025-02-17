// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ANALYSIS_ANALYZE_H
#define EXP_ANALYSIS_ANALYZE_H

#include "imr/function.h"
#include "utility/result.h"

/**
 * @brief infers the type, lifetime of each local variable within the function.
 */
ExpResult analyze_function(Function *function, struct Context *context);

#endif // !EXP_ANALYSIS_ANALYZE_H
