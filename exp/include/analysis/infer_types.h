// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ANALYSIS_INFER_TYPES_H
#define EXP_ANALYSIS_INFER_TYPES_H

#include "imr/function.h"
#include "utility/result.h"

struct Context;
/**
 * @brief Attempts to assign a static type to each local declared in the
 * function and to the function itself.
 *
 * @note this is taken literally, the function has the side effect of ensuring
 * that every local in the function has a non-null static type annotation.
 */
ExpResult infer_types_of_locals(Function *function, struct Context *context);

#endif // !EXP_ANALYSIS_INFER_TYPES_H
