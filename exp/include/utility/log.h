// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_LOG_MESSAGE_H
#define EXP_UTILITY_LOG_MESSAGE_H

#include "utility/io.h"

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
void log_message(LogLevel level, const char *restrict file, u64 line,
                 StringView message, struct File *stream);

#endif // !EXP_UTILITY_LOG_MESSAGE_H
