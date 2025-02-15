// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_PANIC_H
#define EXP_UTILITY_PANIC_H

#include "utility/string_view.h"

[[noreturn]] void panic(StringView msg, const char *file, i32 line);
[[noreturn]] void panic_errno(StringView msg, const char *file, i32 line);

#define PANIC(msg)       panic(SV(msg), __FILE__, __LINE__)
#define PANIC_ERRNO(msg) panic_errno(SV(msg), __FILE__, __LINE__)

#endif // !EXP_UTILITY_PANIC_H
