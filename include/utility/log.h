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
 * @brief logs an internal message to the given stream.
 *
 * an internal message is a warning or error caused by an internal bug or
 * to expose internal behavior of the compiler
 *
 * @note message is formatted as
 * "[<fatal|warning|status> @ <file>:<line>] <message>\n"
 *
 * @param level the severity of the message
 * @param file the filename related to the message
 * @param line the linenum related to the message
 * @param message the message to print
 * @param stream the stream to write to
 */
void log_message(LogLevel level, const char *restrict file, size_t line,
                 const char *restrict message, FILE *restrict stream);

#endif // !EXP_UTILITY_LOG_MESSAGE_H