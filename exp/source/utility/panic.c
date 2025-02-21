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
 * @file utility/panic.c
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "utility/break.h"
#include "utility/log.h"
#include "utility/panic.h"

[[noreturn]] void panic(StringView msg, const char *file, i32 line) {

    log_message(LOG_FATAL, file, (u64)line, msg, program_error);
    EXP_BREAK();
    exit(EXIT_FAILURE);
}

[[noreturn]] void panic_errno(StringView msg, const char *file, i32 line) {

    static char const *text = " errno: ";
    char const *errmsg      = strerror(errno);
    u64 msglen = msg.length, errmsglen = strlen(errmsg), textlen = strlen(text),
        buflen = msglen + errmsglen + textlen;
    char buf[buflen + 1];
    memcpy(buf, msg.ptr, msglen);
    memcpy(buf + msglen, text, textlen);
    memcpy(buf + msglen + textlen, errmsg, errmsglen);
    buf[buflen] = '\0';

    log_message(LOG_FATAL, file, (u64)line, string_view_from_str(buf, buflen),
                program_error);
    EXP_BREAK();
    exit(EXIT_FAILURE);
}
