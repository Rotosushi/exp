/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "utility/syscall/exit.h"
#include "utility/config.h"
#include "utility/unreachable.h"

#ifndef EXP_HOST_SYSTEM_LINUX
    #error "unsupported host os"
#endif

#include <sys/syscall.h>
#include <unistd.h>

[[noreturn]] void sysexit([[maybe_unused]] i32 code) {
    syscall(SYS_exit, code);
    EXP_UNREACHABLE();
}
