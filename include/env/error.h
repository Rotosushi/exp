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

#include "utility/log.h"
#include "utility/string.h"

typedef enum ErrorCode {
  ERROR_NONE,

  ERROR_LEXER_ERROR_UNEXPECTED_CHAR,
  ERROR_LEXER_ERROR_UNMATCHED_DOUBLE_QUOTE,

  ERROR_INTEGER_TO_LARGE,

  ERROR_EXPECTED_SEMICOLON,
  ERROR_EXPECTED_EQUAL,
  ERROR_EXPECTED_IDENTIFIER,
  ERROR_EXPECTED_KEYWORD_CONST,

  ERROR_UNEXPECTED_TOKEN,
} ErrorCode;

char const *error_code_cstring(ErrorCode code);

typedef struct Error {
  ErrorCode code;
  String message;
} Error;

Error error_create();
Error error_construct(ErrorCode code, char const *restrict data);
Error error_from_view(ErrorCode code, StringView sv);
void error_destroy(Error *restrict error);

void error_assign(Error *restrict error, ErrorCode code,
                  char const *restrict data);

void error_print(Error *restrict error, char const *restrict file, size_t line);

#endif // !EXP_ENV_ERROR_H