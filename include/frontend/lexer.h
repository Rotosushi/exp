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
#pragma once
#include <stddef.h>

#include "frontend/token.h"
#include "utility/string_view.h"

typedef struct Lexer {
  const char *token;
  const char *cursor;
  size_t line;
} Lexer;

void lexer_init(Lexer *lexer);
void lexer_reset(Lexer *lexer);
void lexer_set_view(Lexer *lexer, const char *buffer);

bool lexer_at_end(Lexer *lexer);

string_view lexer_current_text(Lexer *lexer);
size_t lexer_current_line(Lexer *lexer);

Token lexer_scan(Lexer *lexer);
