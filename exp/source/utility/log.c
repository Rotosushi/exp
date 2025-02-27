/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
