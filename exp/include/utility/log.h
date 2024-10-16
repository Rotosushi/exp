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

#include "utility/string_view.h"

typedef enum LogLevel {
  LOG_FATAL,
  LOG_ERROR,
  LOG_WARNING,
  LOG_STATUS
} LogLevel;

void write_note(FILE *restrict stream,
                LogLevel level,
                StringView message,
                StringView file,
                u64 line);

#endif // !EXP_UTILITY_LOG_MESSAGE_H
