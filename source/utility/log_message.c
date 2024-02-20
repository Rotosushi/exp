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
#include <stdlib.h>
#include <string.h>

#include "filesystem/io.h"
#include "utility/log_message.h"
#include "utility/numbers_to_string.h"

#define LOG_FATAL_MSG "fatal"
#define LOG_WARNING_MSG "warning"
#define LOG_STATUS_MSG "status"
#define BAD_LOG_LEVEL_MSG "unknown log level"

void log_message(LogLevel log_level, const char *file, size_t file_length,
                 int line, const char *message, size_t message_length,
                 FILE *restrict stream) {
  file_write("[", sizeof("["), stream);

  switch (log_level) {
  case LOG_FATAL:
    file_write(LOG_FATAL_MSG, sizeof(LOG_FATAL_MSG), stream);
    break;

  case LOG_WARNING:
    file_write(LOG_WARNING_MSG, sizeof(LOG_WARNING_MSG), stream);
    break;

  case LOG_STATUS:
    file_write(LOG_STATUS_MSG, sizeof(LOG_STATUS_MSG), stream);
    break;

  default:
    file_write(BAD_LOG_LEVEL_MSG, sizeof(BAD_LOG_LEVEL_MSG), stream);
    abort();
  }

  file_write(" @ ", sizeof(" @ "), stream);

  file_write(file, file_length, stream);

  file_write(":", sizeof(":"), stream);

  size_t line_length = uintmax_safe_strlen((uintmax_t)line, 10);
  char line_buffer[line_length];
  char *number_end = uintmax_to_str((uintmax_t)line, line_buffer, 10);
  file_write(line_buffer, (size_t)(number_end - line_buffer), stream);

  file_write("] ", sizeof("] "), stream);

  file_write(message, message_length, stream);

  file_write("\n", sizeof("\n"), stream);
}

#undef LOG_FATAL_MSG
#undef LOG_WARNING_MSG
#undef LOG_STATUS_MSG
#undef BAD_LOG_LEVEL_MSG