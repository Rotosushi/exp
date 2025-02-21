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
 * @file utility/log.c
 */

#include "utility/log.h"
#include "utility/assert.h"
#include "utility/io.h"
#include "utility/string.h"
#include "utility/unreachable.h"

#define LOG_FATAL_MSG   SV("fatal")
#define LOG_ERROR_MSG   SV("error")
#define LOG_WARNING_MSG SV("warning")
#define LOG_STATUS_MSG  SV("status")
// #define BAD_LOG_LEVEL_MSG "unknown log level"

void log_message(LogLevel level, const char *restrict file, u64 line,
                 StringView message, struct File *stream) {
    EXP_ASSERT(stream != nullptr);
    String buffer;
    string_initialize(&buffer);
    string_append(&buffer, SV("["));

    switch (level) {
    case LOG_FATAL:   string_append(&buffer, LOG_FATAL_MSG); break;
    case LOG_ERROR:   string_append(&buffer, LOG_ERROR_MSG); break;
    case LOG_WARNING: string_append(&buffer, LOG_WARNING_MSG); break;
    case LOG_STATUS:  string_append(&buffer, LOG_STATUS_MSG); break;
    default:          EXP_UNREACHABLE();
    }

    if (file != nullptr) {
        string_append(&buffer, SV(" @ "));
        string_append(&buffer, string_view_from_cstring(file));
        string_append(&buffer, SV(":"));
        string_append_u64(&buffer, line);
    }

    string_append(&buffer, SV("] "));
    string_append(&buffer, message);
    string_append(&buffer, SV("\n"));

    file_write(string_to_view(&buffer), stream);
}

#undef LOG_FATAL_MSG
#undef LOG_WARNING_MSG
#undef LOG_STATUS_MSG
// #undef BAD_LOG_LEVEL_MSG
