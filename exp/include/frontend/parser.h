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
#ifndef EXP_FRONTEND_PARSER_H
#define EXP_FRONTEND_PARSER_H

#include "env/context.h"

/*
  definition = "fn" identifier formal-args "->" type body

  formal-args = "(" formal-arg-list? ")"

  formal-arg-list = formal-arg ("," formal-arg)*

  formal-arg = identifier ":" type

  body = "{" statement* "}"

  statement = "return" expression ";"
            | "const" identifier "=" expression ";"
            | expression ";"

  expression = basic (binop precedece-parser)*

  basic = literal
        | unop basic
        | "(" expression ")"
        | identifier

  literal = integer
          | "true"
          | "false"
          | "()"
*/

/**
 * @brief Parse the buffer
 *
 * @param buffer the buffer to parse
 * @param length the length of the buffer
 * @param context the context to parse within
 * @return i32 EXIT_FAILURE or EXIT_SUCCESS
 */
i32 parse_buffer(char const *restrict buffer,
                 u64 length,
                 Context *restrict context);

i32 parse_source(Context *restrict context);

#endif // !EXP_FRONTEND_PARSER_H
