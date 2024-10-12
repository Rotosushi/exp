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
#include "utility/log.h"
#include "utility/panic.h"

StringView error_code_sv(ErrorCode code) {
  switch (code) {
  case ERROR_NONE: return SV("none");

  case ERROR_PRECISION: return SV("precision");
  case ERROR_SYNTAX:    return SV("syntax");
  case ERROR_SEMANTICS: return SV("semantics");

  default: PANIC("bad ErrorCode");
  }
}

Error error_create() {
  Error error = {.code     = ERROR_NONE,
                 .message  = string_create(),
                 .location = source_location_create()};
  return error;
}

Error error(ErrorCode code, String message, SourceLocation location) {
  Error error = {.code = code, .message = message, .location = location};
  return error;
}

void error_destroy(Error *restrict error) {
  error->code = ERROR_NONE;
  string_destroy(&error->message);
}

void print_error(Error *restrict error, String *restrict out) {
  string_append(out, SV("[#"));
  string_append(out, error_code_sv(error->code));
  print_source_location(out, error->location);
  string_append(out, SV(" "));
  string_append_string(out, &error->message);
  string_append(out, SV("]"));
}

void write_error(Error *restrict error, FILE *restrict out) {
  String buf = string_create();
  print_error(error, &buf);
  write_note(out,
             LOG_ERROR,
             string_to_view(&buf),
             error->location.file,
             error->location.line);
  string_destroy(&buf);
}
