// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_RESULT_H
#define EXP_UTILITY_RESULT_H

#include "utility/int_types.h"

typedef enum ExpResult : i32 {
    EXP_SUCCESS = 0x0,
    EXP_FAILURE = 0x1,
} ExpResult;

#endif // EXP_UTILITY_RESULT_H
