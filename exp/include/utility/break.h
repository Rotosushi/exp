// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_BREAK_H
#define EXP_UTILITY_BREAK_H

#ifndef NDEBUG
    #if defined(__GNUC__) || defined(__clang__)
        #define EXP_BREAK() __builtin_trap()
    #else
        #include <stdlib.h>
        #define EXP_BREAK() abort()
    #endif // if defined(__GNUC__) || defined(__clang__)
#else
    #define EXP_BREAK()
#endif // if EXP_DEBUG

#endif // !EXP_UTILITY_BREAK_H
