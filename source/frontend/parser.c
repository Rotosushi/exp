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
#include <stdlib.h>

#include "frontend/parser.h"

/*
  #NOTE: I am going to implement a subset of the total grammar to begin.
  to wit:

  top = declaration ";"

  declaration = "const" identifier (":" type)? = affix

  affix = basic // (binop precedece-parser)?

  basic = literal
        //| identifier
        //| unop basic
        //| "(" affix ")"

  literal = integer
*/

Parser parser_create() {
  Parser parser;
  parser.lexer = lexer_create();
  parser.curtok = TOK_ERROR;
  return parser;
}

void parser_reset(Parser *restrict parser) {
  lexer_reset(&(parser->lexer));
  parser->curtok = TOK_ERROR;
}

void parser_set_view(Parser *restrict parser, char const *buffer) {
  lexer_set_view(&(parser->lexer), buffer);
}

// int parser_parse(Parser *restrict parser, Context *restrict context) {

//   return EXIT_SUCCESS;
// }