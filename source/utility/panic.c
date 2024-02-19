/**
 * Copyright (C) 2024 Cade Weinberg
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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */

#define __STDC_WANT_LIB_EXT1__ 1
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "utility/config.h"
#include "utility/fileio.h"
#include "utility/panic.h"

[[noreturn]] void panic(const char *msg, size_t msg_len) {
#if EXP_DEBUG
  __builtin_trap();
#endif

  file_write(msg, msg_len, stderr);
  exit(EXIT_FAILURE);
}

[[noreturn]] void panic_errno(const char *msg, size_t msg_len) {
#if EXP_DEBUG
  __builtin_trap();
#endif

  file_write(msg, msg_len, stderr);
  file_write(": ", sizeof(": "), stderr);
  const char *errmsg = strerror(errno);
  file_write(errmsg, strlen(errmsg), stderr);
  exit(EXIT_FAILURE);
}