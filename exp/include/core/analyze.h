// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_CORE_ANALYZE_H
#define EXP_CORE_ANALYZE_H

#include "env/context.h"
#include "utility/result.h"

/**
 * @brief Perform Type Inference, Type Checking, and Lifetime Analysis
 * on all functions within the given context.
 *
 *
 */
ExpResult analyze_context(Context *context);

#endif // EXP_CORE_ANALYZE_H
