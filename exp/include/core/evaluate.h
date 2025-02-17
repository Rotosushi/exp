// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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
