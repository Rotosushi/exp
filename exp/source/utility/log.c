/**
 * Copyright (C) 2024 cade
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utility/io.h"
#include "utility/log.h"

#define LOG_FATAL_MSG     "fatal"
#define LOG_ERROR_MSG     "error"
#define LOG_WARNING_MSG   "warning"
#define LOG_STATUS_MSG    "status"
#define BAD_LOG_LEVEL_MSG "unknown log level"

void write_log_level(FILE *restrict out, LogLevel level) {
  switch (level) {
  case LOG_FATAL:   file_write(out, SV(LOG_FATAL_MSG)); break;
  case LOG_ERROR:   file_write(out, SV(LOG_ERROR_MSG)); break;
  case LOG_WARNING: file_write(out, SV(LOG_WARNING_MSG)); break;
  case LOG_STATUS:  file_write(out, SV(LOG_STATUS_MSG)); break;
  default:          file_write(out, SV(BAD_LOG_LEVEL_MSG)); break;
  }
}

void write_note(FILE *restrict out,
                LogLevel level,
                StringView message,
                StringView file,
                u64 line) {
  file_write(out, SV("[#"));
  write_log_level(out, level);

  if (!string_view_empty(file)) {
    file_write(out, SV(" @"));
    file_write(out, file);
    file_write(out, SV(":"));
    file_write_u64(out, line);
  }

  file_write(out, SV("] "));
  file_write(out, message);
  file_write(out, SV("\n"));
}

#undef LOG_FATAL_MSG
#undef LOG_WARNING_MSG
#undef LOG_STATUS_MSG
#undef BAD_LOG_LEVEL_MSG
