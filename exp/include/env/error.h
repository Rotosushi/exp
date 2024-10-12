// Copyright (C) 2024 Cade Weinberg
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
#ifndef EXP_ENV_ERROR_H
#define EXP_ENV_ERROR_H

#include "adt/string.h"
#include "frontend/source_location.h"

typedef enum ErrorCode : u8 {
  ERROR_NONE,

  ERROR_PRECISION,
  ERROR_SYNTAX,
  ERROR_SEMANTICS,
} ErrorCode;

StringView error_code_sv(ErrorCode code);

/**
 * @brief models errors which are produced during compilation.
 * designed to allow the creation of informative and readable
 * error messages to users of the compiler.
 *
 * @note this is not intended to model errors that are internal
 * to the compiler, and signal code defects within the compiler.
 * The general strategy for internal errors is to call panic.
 *
 */
typedef struct Error {
  ErrorCode code;
  String message;
  SourceLocation location;
} Error;

Error error_create();
Error error(ErrorCode code, String message, SourceLocation location);
void error_destroy(Error *restrict error);

void print_error(Error *restrict error, String *restrict out);
void write_error(Error *restrict error, FILE *restrict out);

#endif // !EXP_ENV_ERROR_H
