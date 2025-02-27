/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
