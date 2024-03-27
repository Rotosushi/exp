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

typedef enum LogLevel {
  LOG_FATAL,
  LOG_ERROR,
  LOG_WARNING,
  LOG_STATUS
} LogLevel;

/**
 * @brief logs a message to the given stream
 *
 * @note log_message is formatted as
 * "[<fatal|warning|status> @ <file>:<line>] <message>\n"
 *
 * @param log_level the severity of the message
 * @param file the filename the log_message originated from
 * @param file_length the length of the filename
 * @param line the linenum the log_message originated from
 * @param message the message to print
 * @param message_length the length of the message
 * @param stream the stream to write to
 */
void log_message(LogLevel log_level, const char *file, int line,
                 const char *message, FILE *restrict stream);

#define LOG(log_level, message, stream)                                        \
  log_message(log_level, __FILE__, __LINE__, message, stream)

#endif // !EXP_UTILITY_LOG_MESSAGE_H