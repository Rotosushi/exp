// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_SYSCALL_EXIT_H
#define EXP_UTILITY_SYSCALL_EXIT_H

#include "utility/int_types.h"

[[noreturn]] void sysexit(i32 code);

#endif // EXP_UTILITY_SYSCALL_EXIT_H
