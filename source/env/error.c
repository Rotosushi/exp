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
#include "env/error.h"
#include "utility/panic.h"

char const *error_code_cstring(ErrorCode code) {
  switch (code) {
  case ERROR_NONE:
    return "none";
  case ERROR_INTEGER_TO_LARGE:
    return "Integer literal too large.";

  case ERROR_EXPECTED_IDENTIFIER:
    return "Expected an Identifier. Found: ";

  case ERROR_EXPECTED_SEMICOLON:
    return "Expected: [;]. Found: ";
  case ERROR_EXPECTED_EQUAL:
    return "Expected: [=]. Found: ";
  case ERROR_EXPECTED_KEYWORD_CONST:
    return "Expected: [const]. Found: ";

  case ERROR_UNEXPECTED_TOKEN:
    return "Unexpected Token: ";

  default:
    panic("bad ErrorCode");
  }
}

Error error_create() {
  Error error;
  error.code = ERROR_NONE;
  error.message = string_create();
  return error;
}

Error error_construct(ErrorCode code, char const *restrict data) {
  Error error;
  error.code = code;
  string_assign(&error.message, data);
  return error;
}

Error error_from_view(ErrorCode code, StringView sv) {
  Error error;
  error.code = code;
  error.message = string_from_view(sv);
  return error;
}

void error_destroy(Error *restrict error) {
  error->code = ERROR_NONE;
  string_destroy(&error->message);
}

void error_assign(Error *restrict error, ErrorCode code,
                  char const *restrict data) {
  error->code = code;
  string_assign(&error->message, data);
}