// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ANALYSIS_LIFETIMES_H
#define EXP_ANALYSIS_LIFETIMES_H

#include "imr/function.h"

struct Context;
/**
 * @brief infers the lifetime of each local variable within the function.
 */
void analyze_lifetimes_of_locals(Function *function, struct Context *context);

#endif // EXP_ANALYSIS_LIFETIMES_H
