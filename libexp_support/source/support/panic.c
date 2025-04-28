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

#include "support/ansi_colors.h"
#include "support/assert.h"
#include "support/debug.h"
#include "support/message.h"
#include "support/panic.h"

[[noreturn]] void panic(StringView msg, const char *file, i32 line) {
    exp_assert(file != NULL);
    u64  redlen = sizeof(ANSI_COLOR_RED) - 1;
    u64  rstlen = sizeof(ANSI_COLOR_RESET) - 1;
    u64  msglen = msg.length + redlen + rstlen;
    char msgbuf[msglen + 1];
    u64  offset = 0;
    memcpy(msgbuf + offset, ANSI_COLOR_RED, redlen);
    offset += redlen;
    memcpy(msgbuf + offset, msg.ptr, msg.length);
    offset += msg.length;
    memcpy(msgbuf + offset, ANSI_COLOR_RESET, rstlen);
    msgbuf[msglen] = '\0';
    message(
        MESSAGE_FATAL, file, (u64)line, string_view(msgbuf, msglen), stderr);
    EXP_BREAK();
    exit(EXIT_FAILURE);
}

[[noreturn]] void panic_errno(StringView msg, const char *file, i32 line) {
    exp_assert(file != NULL);
    static char const text[]    = " :: ";
    char const       *errmsg    = strerror(errno);
    u64               redlen    = sizeof(ANSI_COLOR_RED) - 1;
    u64               rstlen    = sizeof(ANSI_COLOR_RED) - 1;
    u64               errmsglen = strlen(errmsg);
    u64               textlen   = sizeof(text) - 1;
    u64               buflen =
        redlen + msg.length + rstlen + textlen + redlen + errmsglen + rstlen;
    u64  offset = 0;
    char msgbuf[buflen + 1];
    memcpy(msgbuf + offset, ANSI_COLOR_RED, redlen);
    offset += redlen;
    memcpy(msgbuf + offset, msg.ptr, msg.length);
    offset += msg.length;
    memcpy(msgbuf + offset, ANSI_COLOR_RESET, rstlen);
    offset += rstlen;
    memcpy(msgbuf + offset, text, textlen);
    offset += textlen;
    memcpy(msgbuf + offset, ANSI_COLOR_RED, redlen);
    offset += redlen;
    memcpy(msgbuf + offset, errmsg, errmsglen);
    offset += errmsglen;
    memcpy(msgbuf + offset, ANSI_COLOR_RESET, rstlen);
    offset += rstlen;
    msgbuf[buflen] = '\0';

    message(
        MESSAGE_FATAL, file, (u64)line, string_view(msgbuf, buflen), stderr);
    EXP_BREAK();
    exit(EXIT_FAILURE);
}
