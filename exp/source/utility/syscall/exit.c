/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file utility/syscall/exit.c
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
