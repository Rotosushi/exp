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
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

#define LOG_FATAL_MSG     "fatal"
#define LOG_ERROR_MSG     "error"
#define LOG_WARNING_MSG   "warning"
#define LOG_STATUS_MSG    "status"
#define BAD_LOG_LEVEL_MSG "unknown log level"

void log_message(LogLevel level, const char *restrict file, u64 line,
                 const char *restrict message, FILE *restrict stream) {
  file_write("[", stream);

  switch (level) {
  case LOG_FATAL:
    file_write(LOG_FATAL_MSG, stream);
    break;

  case LOG_ERROR:
    file_write(LOG_ERROR_MSG, stream);
    break;

  case LOG_WARNING:
    file_write(LOG_WARNING_MSG, stream);
    break;

  case LOG_STATUS:
    file_write(LOG_STATUS_MSG, stream);
    break;

  default:
    file_write(BAD_LOG_LEVEL_MSG, stream);
    abort();
  }

  if (file != NULL) {
    file_write(" @ ", stream);

    file_write(file, stream);

    file_write(":", stream);

    print_u64(line, RADIX_DECIMAL, stream);
  }

  file_write("] ", stream);

  file_write(message, stream);

  file_write("\n", stream);
}

#undef LOG_FATAL_MSG
#undef LOG_WARNING_MSG
#undef LOG_STATUS_MSG
#undef BAD_LOG_LEVEL_MSG