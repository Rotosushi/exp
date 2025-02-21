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
 * @file frontend/parser.h
 */

#ifndef EXP_FRONTEND_PARSER_H
#define EXP_FRONTEND_PARSER_H

#include "env/context.h"
#include "utility/result.h"

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
        | "(" expression ("," expression)* ")"
        | identifier

  literal = integer
          | "true"
          | "false"
          | "()"
*/

ExpResult parse_expression(StringView source, Context *context);

#endif // !EXP_FRONTEND_PARSER_H
