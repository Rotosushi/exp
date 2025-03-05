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

#include "support/int_types.h"
#include "support/string_view.h"

typedef enum LogLevel {
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_STATUS
} LogLevel;

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
void log_message(LogLevel level,
                 const char *restrict file,
                 u64 line,
                 StringView message,
                 FILE *restrict stream);

#endif // !EXP_UTILITY_LOG_MESSAGE_H
