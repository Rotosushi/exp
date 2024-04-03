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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "env/error.h"
#include "frontend/parser.h"
#include "utility/numbers_to_string.h"
#include "utility/panic.h"

typedef struct Parser {
  Lexer lexer;
  Token curtok;
} Parser;

typedef struct MaybeError {
  bool has_error;
  Error error;
} MaybeError;

static MaybeError success() {
  MaybeError me;
  me.has_error = 0;
  me.error = error_create();
  return me;
}

static MaybeError error(Parser *restrict parser, ErrorCode code) {
  MaybeError me;
  me.has_error = 0;
  me.error = error_from_view(code, lexer_current_text(&parser->lexer));
  return me;
}

/*
  #NOTE: I am going to implement a subset of the total grammar to begin.
  to wit:

  top = declaration ";"

  declaration = "const" identifier = affix

  affix = basic // (binop precedece-parser)?

  basic = integer
      //| "true" | "false"
      //| "nil"
      //| identifier
      //| unop basic
      //| "(" affix ")"
*/

static Parser parser_create() {
  Parser parser;
  parser.lexer = lexer_create();
  parser.curtok = TOK_END;
  return parser;
}

// static void parser_reset(Parser *restrict parser) {
//   assert(parser != NULL);
//   lexer_reset(&(parser->lexer));
//   parser->curtok = TOK_ERROR;
// }

static void parser_set_view(Parser *restrict parser, char const *buffer) {
  assert(parser != NULL);
  assert(buffer != NULL);
  lexer_set_view(&(parser->lexer), buffer);
}

static bool finished(Parser *restrict parser) {
  return lexer_at_end(&parser->lexer);
}

static StringView curtxt(Parser *restrict parser) {
  return lexer_current_text(&parser->lexer);
}

static size_t curline(Parser *restrict parser) {
  return lexer_current_line(&parser->lexer);
}

// static size_t curcol(Parser *restrict parser) {
//   return lexer_current_column(&parser->lexer);
// }

static void nexttok(Parser *restrict parser) {
  parser->curtok = lexer_scan(&parser->lexer);
}

static bool peek(Parser *restrict parser, Token token) {
  return parser->curtok == token;
}

static bool expect(Parser *restrict parser, Token token) {
  if (peek(parser, token)) {
    nexttok(parser);
    return 1;
  } else {
    return 0;
  }
}

static MaybeError parse_basic(Parser *restrict parser,
                              Context *restrict context) {
  switch (parser->curtok) {
  case TOK_NIL: {
    size_t index = context_constants_append(context, value_create_nil());
    context_emit_push_constant(context, index);
    nexttok(parser);
    return success();
  }

  case TOK_TRUE: {
    size_t index = context_constants_append(context, value_create_boolean(1));
    context_emit_push_constant(context, index);
    nexttok(parser);
    return success();
  }

  case TOK_FALSE: {
    size_t index = context_constants_append(context, value_create_boolean(0));
    context_emit_push_constant(context, index);
    nexttok(parser);
    return success();
  }

  case TOK_INTEGER: {
    StringView sv = curtxt(parser);
    long integer = strtol(sv.ptr, NULL, 10);
    if (errno == ERANGE) {
      return error(parser, ERROR_INTEGER_TO_LARGE);
    }

    size_t index =
        context_constants_append(context, value_create_integer(integer));
    context_emit_push_constant(context, index);
    nexttok(parser);
    return success();
  }

  case TOK_STRING_LITERAL: {
    StringView sl = context_intern(context, curtxt(parser));
    size_t index =
        context_constants_append(context, value_create_string_literal(sl));
    context_emit_push_constant(context, index);
    nexttok(parser);
    return success();
  }

  case TOK_ERROR_UNMATCHED_DOUBLE_QUOTE: {
    return error(parser, ERROR_LEXER_ERROR_UNMATCHED_DOUBLE_QUOTE);
  }

  case TOK_ERROR_UNEXPECTED_CHAR: {
    return error(parser, ERROR_LEXER_ERROR_UNEXPECTED_CHAR);
  }

  default:
    return error(parser, ERROR_UNEXPECTED_TOKEN);
  }
}

static MaybeError parse_affix(Parser *restrict parser,
                              Context *restrict context) {
  return parse_basic(parser, context);
}

static MaybeError parse_declaration(Parser *restrict parser,
                                    Context *restrict context) {
  if (!expect(parser, TOK_CONST)) {
    return error(parser, ERROR_EXPECTED_KEYWORD_CONST);
  }

  if (!peek(parser, TOK_IDENTIFIER)) {
    return error(parser, ERROR_EXPECTED_IDENTIFIER);
  }

  StringView name = context_intern(context, curtxt(parser));
  size_t index =
      context_constants_append(context, value_create_string_literal(name));
  context_emit_push_constant(context, index);

  nexttok(parser);

  if (!expect(parser, TOK_EQUAL)) {
    return error(parser, ERROR_EXPECTED_EQUAL);
  }

  MaybeError maybe = parse_affix(parser, context);
  if (maybe.has_error) {
    return maybe;
  }

  if (!expect(parser, TOK_SEMICOLON)) {
    return error(parser, ERROR_EXPECTED_SEMICOLON);
  }

  context_emit_define_global_constant(context);
  return success();
}

static MaybeError parse_top(Parser *restrict parser,
                            Context *restrict context) {
  return parse_declaration(parser, context);
}

int parse(char const *restrict buffer, Context *restrict context) {
  assert(buffer != NULL);
  assert(context != NULL);

  Parser parser = parser_create();

  parser_set_view(&parser, buffer);
  nexttok(&parser);

  while (!finished(&parser)) {
    MaybeError maybe = parse_top(&parser, context);
    if (maybe.has_error) {
      StringView source = context_source_path(context);
      error_print(&maybe.error, source.ptr, curline(&parser));
      return EXIT_FAILURE;
    }
  }

  context_emit_stop(context);

  return EXIT_SUCCESS;
}

int parse_source(Context *restrict context) {
  assert(context != NULL);
  String buffer = context_buffer_source(context);
  int result = parse(buffer.buffer, context);
  string_destroy(&buffer);
  return result;
}