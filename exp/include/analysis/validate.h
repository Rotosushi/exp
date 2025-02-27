// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ANALYSIS_VALIDATE_H
#define EXP_ANALYSIS_VALIDATE_H

#include "imr/function.h"
#include "utility/result.h"

/**
 * @brief asserts that the function is correct in so far as the IR in
 * concerned.
 *
 * @note asserts the following:
 * - each local has a valid static type.
 * - each local has a valid lifetime.
 * - each instruction using a local uses it in a typesafe manner.
 * - each instruction using a value uses it in a typesafe manner.
 * - each instruction using a label uses it in a typesafe manner.
 */
ExpResult validate_function(Function *function, struct Context *context);

#endif // EXP_ANALYSIS_VALIDATE_H
