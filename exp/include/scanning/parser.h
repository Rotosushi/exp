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
#include "scanning/lexer.h"

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

/* #TODO: I want to change the language again. (I know):

  top = let
      | expression ";"

  let = "let" (":" type)? "=" expression ";"

  expression = basic (binop precedece-parser)*

  basic = literal
        | unop basic
        | "(" expression ("," expression)* ")"
        | "\" formal-args ("->" type) body
        | identifier

  literal = integer
          | "true"
          | "false"
          | "()"

  formal-args = "(" formal-arg-list? ")"

  formal-arg-list = formal-arg ("," formal-arg)*

  formal-arg = identifier ":" type

  body = "{" statement* "}"

  statement = return
            | let
            | expression ";"

  return = "return" expression ";"
*/

typedef struct Parser {
    Context  *context;
    Function *function;
    Lexer     lexer;
    Token     curtok;
} Parser;

void parser_create(Parser *restrict parser, Context *restrict context);

bool parser_setup(Parser *restrict parser, StringView view);

void parser_set_file(Parser *restrict parser, StringView file);
void parser_current_source_location(Parser const *restrict parser,
                                    SourceLocation *restrict source_location);

bool parser_done(Parser const *restrict parser);

bool parser_parse_expression(Parser *restrict parser,
                             Function *restrict expression);

#endif // !EXP_FRONTEND_PARSER_H
