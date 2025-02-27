// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_UNREACHABLE_H
#define EXP_UTILITY_UNREACHABLE_H

#ifndef NDEBUG
    #include "utility/panic.h"
    #define EXP_UNREACHABLE() PANIC("unreachable")
#else
    #include <stddef.h>
    #define EXP_UNREACHABLE() unreachable()
#endif // NDEBUG

#endif // !EXP_UTILITY_UNREACHABLE_H
