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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "utility/debug.h"
#include "utility/log.h"
#include "utility/panic.h"

[[noreturn]] void panic(StringView msg, const char *file, i32 line) {
  EXP_BREAK()
  write_note(stderr, LOG_FATAL, msg, string_view_from_cstring(file), (u64)line);
  exit(EXIT_FAILURE);
}

[[noreturn]] void panic_errno(StringView msg, const char *file, i32 line) {
  EXP_BREAK()

  static char const *text = " errno: ";
  char const *errmsg      = strerror(errno);
  u64 msglen = msg.length, errmsglen = strlen(errmsg), textlen = strlen(text),
      buflen = msglen + errmsglen + textlen;
  char buf[buflen + 1];
  memcpy(buf, msg.ptr, msglen);
  memcpy(buf + msglen, text, textlen);
  memcpy(buf + msglen + textlen, errmsg, errmsglen);
  buf[buflen] = '\0';

  write_note(stderr, LOG_FATAL, msg, string_view_from_cstring(file), (u64)line);
  exit(EXIT_FAILURE);
}
