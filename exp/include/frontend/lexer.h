// Copyright (C) 2025 Cade Weinberg
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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file frontend/lexer.h
 */

#ifndef EXP_FRONTEND_LEXER_H
#define EXP_FRONTEND_LEXER_H

#include <stddef.h>

#include "frontend/token.h"
#include "utility/string_view.h"

typedef struct Lexer {
    const char *buffer;
    const char *token;
    const char *cursor;
    u64 line;
    u64 column;
    u64 length;
} Lexer;

/**
 * @brief create a new lexer
 *
 * @return Lexer
 */
Lexer lexer_create();

/**
 * @brief initialize a new lexer
 *
 * @param lexer
 */
void lexer_init(Lexer *restrict lexer);

/**
 * @brief reset a lexer to is initialized state
 *
 * @param lexer
 */
void lexer_reset(Lexer *restrict lexer);

/**
 * @brief set the buffer the lexer scans
 *
 * @note the lexer expects the buffer to be null terminated
 *
 * @param lexer
 * @param buffer
 */
void lexer_set_view(Lexer *restrict lexer, char const *buffer, u64 length);

/**
 * @brief returns true when the <lexer> has scanned all
 * of it's buffer
 *
 * @param lexer
 * @return true
 * @return false
 */
bool lexer_at_end(Lexer *restrict lexer);

StringView lexer_current_text(Lexer const *restrict lexer);
u64 lexer_current_line(Lexer const *restrict lexer);
u64 lexer_current_column(Lexer const *restrict lexer);

Token lexer_scan(Lexer *restrict lexer);

#endif // !EXP_FRONTEND_LEXER_H
