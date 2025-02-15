// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_PROCESS_H
#define EXP_UTILITY_PROCESS_H
#include "utility/int_types.h"

/**
 * @brief fork/execvp the given file, passing args
 *
 * @warning args must have a NULL at the end
 *
 * @param file
 * @param args
 * @return i32
 */
i32 process(char const *file, char const *args[]);

#endif // !EXP_UTILITY_PROCESS_H
