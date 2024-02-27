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

#include "frontend/lexer.h"

/*
  #NOTE: I am going to implement a subset of the total grammar to begin.
  to wit:

  top = declaration ";"

  declaration = "const" identifier (":" type)? = affix

  affix = basic // (binop precedece-parser)?

  basic = literal
        | identifier
        //| unop basic
        //| "(" affix ")"

  literal = integer
*/

typedef struct Parser {
  Lexer lexer;
  Token curtok;
} Parser;

/**
 * @brief return a new parser
 *
 * @return Parser
 */
Parser parser_create();

/**
 * @brief destroy a parser
 *
 * @param parser
 */
void parser_destroy(Parser *restrict parser);

/**
 * @brief reset a parser
 *
 * @param parser
 */
void parser_reset(Parser *restrict parser);

/**
 * @brief set the text the parser will parse
 *
 * @param parser
 * @param view
 */
void parser_set_view(Parser *restrict parser, StringView view);

/**
 * @brief Parse the source
 *
 * @param parser
 * @param bytecode
 * @return int
 */
int parser_parse(Parser *restrict parser);

#endif // !EXP_FRONTEND_PARSER_H