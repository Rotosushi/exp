// Copyright (C) 2024 cade
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_UTILITY_LOG_MESSAGE_H
#define EXP_UTILITY_LOG_MESSAGE_H

#include <stdio.h>

#include "support/scalar.h"
#include "support/string_view.h"

typedef enum MessageLevel {
    MESSAGE_FATAL,
    MESSAGE_ERROR,
    MESSAGE_WARNING,
    MESSAGE_STATUS,
    MESSAGE_TRACE,
} MessageLevel;

/**
 * @brief logs a message to the given stream.
 *
 * @note message is formatted as
 * "[<fatal|error|warning|status> (@ <file>:<line>)?] <message>\n"
 *
 * @param level the severity of the message
 * @param file the filename related to the message (can be NULL)
 * @param line the linenum related to the message
 * @param message the message to print
 * @param stream the stream to write to
 */
void message(MessageLevel level,
             const char *restrict file,
             u64        line,
             StringView msg,
             FILE *restrict stream);

/**
 * @brief wrapper for message that does not take a file and line number.
 * and uses MESSAGE_TRACE as the MessageLevel.
 */
void trace(StringView msg, FILE *restrict stream);
void trace_u64(u64 value, FILE *restrict stream);
void trace_i64(i64 value, FILE *restrict stream);
void trace_command(StringView   cmd,
                   i32          argc,
                   char const **argv,
                   FILE *restrict stream);

#endif // !EXP_UTILITY_LOG_MESSAGE_H
