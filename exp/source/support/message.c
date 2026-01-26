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

#include "support/ansi_colors.h"
#include "support/assert.h"
#include "support/io.h"
#include "support/message.h"

#define LOG_FATAL_MSG   SV(ANSI_COLOR_RED "fatal" ANSI_COLOR_RESET)
#define LOG_ERROR_MSG   SV(ANSI_COLOR_RED "error" ANSI_COLOR_RESET)
#define LOG_WARNING_MSG SV(ANSI_COLOR_YELLOW "warning" ANSI_COLOR_RESET)
#define LOG_STATUS_MSG  SV(ANSI_COLOR_BLUE "status" ANSI_COLOR_RESET)
#define LOG_TRACE_MSG   SV(ANSI_COLOR_CYAN "trace" ANSI_COLOR_RESET)
#define BAD_LOG_LEVEL_MSG                                                      \
    SV(ANSI_COLOR_RED "unknown log level" ANSI_COLOR_RESET)

void message(MessageLevel level, char const *message, FILE *restrict stream) {
    EXP_ASSERT(stream != NULL);
    file_write(SV("["), stream);

    switch (level) {
    case MESSAGE_FATAL:   file_write(LOG_FATAL_MSG, stream); break;
    case MESSAGE_ERROR:   file_write(LOG_ERROR_MSG, stream); break;
    case MESSAGE_WARNING: file_write(LOG_WARNING_MSG, stream); break;
    case MESSAGE_STATUS:  file_write(LOG_STATUS_MSG, stream); break;
    case MESSAGE_TRACE:   file_write(LOG_TRACE_MSG, stream); break;
    default:              file_write(BAD_LOG_LEVEL_MSG, stream); abort();
    }

    file_write(SV("] "), stream);
    file_write(string_view_from_cstring(message), stream);
    file_write(SV("\n"), stream);
}

#undef LOG_FATAL_MSG
#undef LOG_WARNING_MSG
#undef LOG_STATUS_MSG
#undef LOG_ERROR_MSG
#undef LOG_TRACE_MSG
#undef BAD_LOG_LEVEL_MSG
