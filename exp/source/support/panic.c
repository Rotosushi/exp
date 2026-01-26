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
#include "support/numeric_conversions.h"
#include "support/panic.h"
#include "support/stacktrace.h"

[[noreturn]] void
panic(char const *msg, char const *function, char const *file, long line) {
    EXP_ASSERT(msg != NULL);
    EXP_ASSERT(function != NULL);
    EXP_ASSERT(file != NULL);
    // file @ function:line message
    char const *colon   = ":";
    char const *space   = " ";
    char const *at      = " @ ";
    u64         redlen  = sizeof(ANSI_COLOR_RED) - 1;
    u64         rstlen  = sizeof(ANSI_COLOR_RESET) - 1;
    u64         msglen  = strlen(msg);
    u64         funlen  = strlen(function);
    u64         filelen = strlen(file);
    u64         linelen = i64_safe_strlen(line);
    u64  buflen = msglen + funlen + filelen + linelen + redlen + rstlen + 5;
    char msgbuf[buflen + 1];
    u64  offset = 0;
    memcpy(msgbuf + offset, file, filelen);
    offset += filelen;
    memcpy(msgbuf + offset, at, 3);
    offset += 3;
    memcpy(msgbuf + offset, function, funlen);
    offset += funlen;
    memcpy(msgbuf + offset, colon, 1);
    offset += 1;
    i64_to_str(line, msgbuf);
    offset += linelen;
    memcpy(msgbuf + offset, space, 1);
    offset += 1;
    memcpy(msgbuf + offset, ANSI_COLOR_RED, redlen);
    offset += redlen;
    memcpy(msgbuf + offset, msg, msglen);
    offset += msglen;
    memcpy(msgbuf + offset, ANSI_COLOR_RESET, rstlen);
    msgbuf[msglen] = '\0';

    message(MESSAGE_FATAL, msgbuf, stderr);
    print_trace(stderr);
    EXP_BREAK();
    exit(EXIT_FAILURE);
}

[[noreturn]] void panic_errno(char const *msg,
                              char const *function,
                              char const *file,
                              long        line) {
    EXP_ASSERT(msg != NULL);
    EXP_ASSERT(function != NULL);
    EXP_ASSERT(file != NULL);
    // file @ function:line errstr message
    char const *colon   = ":";
    char const *space   = " ";
    char const *at      = " @ ";
    char const *errstr  = strerror(errno);
    u64         errlen  = strlen(errstr);
    u64         redlen  = sizeof(ANSI_COLOR_RED) - 1;
    u64         rstlen  = sizeof(ANSI_COLOR_RESET) - 1;
    u64         msglen  = strlen(msg);
    u64         funlen  = strlen(function);
    u64         filelen = strlen(file);
    u64         linelen = i64_safe_strlen(line);
    u64         buflen =
        msglen + funlen + filelen + linelen + errlen + redlen + rstlen + 5;
    char msgbuf[buflen + 1];
    u64  offset = 0;
    memcpy(msgbuf + offset, file, filelen);
    offset += filelen;
    memcpy(msgbuf + offset, at, 3);
    offset += 3;
    memcpy(msgbuf + offset, function, funlen);
    offset += funlen;
    memcpy(msgbuf + offset, colon, 1);
    offset += 1;
    i64_to_str(line, msgbuf);
    offset += linelen;
    memcpy(msgbuf + offset, space, 1);
    offset += 1;
    memcpy(msgbuf + offset, errstr, errlen);
    offset += errlen;
    memcpy(msgbuf + offset, ANSI_COLOR_RED, redlen);
    offset += redlen;
    memcpy(msgbuf + offset, msg, msglen);
    offset += msglen;
    memcpy(msgbuf + offset, ANSI_COLOR_RESET, rstlen);
    msgbuf[msglen] = '\0';

    message(MESSAGE_FATAL, msgbuf, stderr);

    print_trace(stderr);
    EXP_BREAK();
    exit(EXIT_FAILURE);
}
