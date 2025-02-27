// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_ASSERT_H
#define EXP_UTILITY_ASSERT_H

#include "utility/config.h"

#ifndef EXP_REMOVE_ASSERTS
    #include "utility/break.h"
    #include "utility/panic.h"
    #define EXP_ASSERT(expression) ((expression) || (PANIC(#expression), 1))
#else
    #define EXP_ASSERT(expression)
#endif

#endif // EXP_UTILITY_ASSERT_H
