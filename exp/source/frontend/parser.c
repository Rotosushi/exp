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
#include "imr/ast.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

typedef struct Parser {
  Lexer lexer;
  Token curtok;
} Parser;

typedef struct ParserResult {
  bool has_error;
  union {
    Ast *ast;
    Error error;
  };
} ParserResult;

static void parser_result_destroy(ParserResult *restrict pr) {
  if (pr->has_error) {
    error_destroy(&pr->error);
  } else {
    ast_destroy(pr->ast);
  }
}

static ParserResult error(Parser *restrict parser, ErrorCode code) {
  ParserResult result;
  result.has_error = 1;
  result.error     = error_from_view(code, lexer_current_text(&parser->lexer));
  return result;
}

static ParserResult success(Ast *restrict ast) {
  ParserResult result;
  result.has_error = 0;
  result.ast       = ast;
  return result;
}

typedef enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY,
} Precedence;

typedef ParserResult (*PrefixFunction)(Parser *restrict p, Context *restrict c);
typedef ParserResult (*InfixFunction)(Parser *restrict p, Context *restrict c,
                                      Ast *restrict left);

typedef struct ParseRule {
  PrefixFunction prefix;
  InfixFunction infix;
  Precedence precedence;
} ParseRule;

static Parser parser_create() {
  Parser parser;
  parser.lexer  = lexer_create();
  parser.curtok = TOK_END;
  return parser;
}

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

static u64 curline(Parser *restrict parser) {
  return lexer_current_line(&parser->lexer);
}

// static u64 curcol(Parser *restrict parser) {
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

static ParseRule *get_rule(Token token);
static ParserResult expression(Parser *restrict parser,
                               Context *restrict context);
static ParserResult parse_precedence(Parser *restrict parser,
                                     Context *restrict context,
                                     Precedence precedence);

static ParserResult constant(Parser *restrict parser,
                             Context *restrict context) {
  nexttok(parser); // eat 'const'

  if (!peek(parser, TOK_IDENTIFIER)) {
    return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }
  StringView name = context_intern(context, curtxt(parser));
  nexttok(parser);

  if (!expect(parser, TOK_EQUAL)) {
    return error(parser, ERROR_PARSER_EXPECTED_EQUAL);
  }

  ParserResult maybe = expression(parser, context);
  if (maybe.has_error) {
    return maybe;
  }

  if (!expect(parser, TOK_SEMICOLON)) {
    return error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
  }

  return success(ast_create_constant(name, maybe.ast));
}

static ParserResult definition(Parser *restrict parser,
                               Context *restrict context) {
  switch (parser->curtok) {
  case TOK_CONST:
    return constant(parser, context);

  default:
    return error(parser, ERROR_PARSER_EXPECTED_KEYWORD_CONST);
  }
}

static ParserResult parens(Parser *restrict parser, Context *restrict context) {
  nexttok(parser); // eat '('

  ParserResult maybe = expression(parser, context);
  if (maybe.has_error) {
    return maybe;
  }

  if (!expect(parser, TOK_END_PAREN)) {
    return error(parser, ERROR_PARSER_EXPECTED_END_PAREN);
  }

  return success(maybe.ast);
}

static ParserResult unop(Parser *restrict parser, Context *restrict context) {
  Token op = parser->curtok;
  nexttok(parser);

  ParserResult maybe = parse_precedence(parser, context, PREC_UNARY);
  if (maybe.has_error) {
    return maybe;
  }

  switch (op) {
  case TOK_MINUS:
    return success(ast_create_unop(OPR_SUB, maybe.ast));

  default:
    unreachable();
  }
}

static ParserResult binop(Parser *restrict parser, Context *restrict context,
                          Ast *restrict left) {
  Token op        = parser->curtok;
  ParseRule *rule = get_rule(op);
  nexttok(parser); // eat the operator

  ParserResult maybe =
      parse_precedence(parser, context, (Precedence)(rule->precedence + 1));
  if (maybe.has_error) {
    return maybe;
  }

  switch (op) {
  case TOK_PLUS:
    return success(ast_create_binop(OPR_ADD, left, maybe.ast));

  case TOK_MINUS:
    return success(ast_create_binop(OPR_SUB, left, maybe.ast));

  case TOK_STAR:
    return success(ast_create_binop(OPR_MUL, left, maybe.ast));

  case TOK_SLASH:
    return success(ast_create_binop(OPR_DIV, left, maybe.ast));

  case TOK_PERCENT:
    return success(ast_create_binop(OPR_MOD, left, maybe.ast));

  default:
    unreachable();
  }
}

static ParserResult nil(Parser *restrict parser,
                        [[maybe_unused]] Context *restrict context) {
  nexttok(parser);
  return success(ast_create_value(value_create_nil()));
}

static ParserResult boolean_true(Parser *restrict parser,
                                 [[maybe_unused]] Context *restrict context) {
  nexttok(parser);
  return success(ast_create_value(value_create_boolean(1)));
}

static ParserResult boolean_false(Parser *restrict parser,
                                  [[maybe_unused]] Context *restrict context) {
  nexttok(parser);
  return success(ast_create_value(value_create_boolean(0)));
}

static ParserResult integer(Parser *restrict parser,
                            [[maybe_unused]] Context *restrict context) {
  StringView sv = curtxt(parser);
  i64 integer   = strtol(sv.ptr, NULL, 10);
  if (errno == ERANGE) {
    return error(parser, ERROR_PARSER_INTEGER_TO_LARGE);
  }

  nexttok(parser);
  return success(ast_create_value(value_create_integer(integer)));
}

static ParserResult expression(Parser *restrict parser,
                               Context *restrict context) {
  return parse_precedence(parser, context, PREC_ASSIGNMENT);
}

static ParserResult parse_precedence(Parser *restrict parser,
                                     Context *restrict context,
                                     Precedence precedence) {
  ParseRule *rule = get_rule(parser->curtok);
  if (rule->prefix == NULL) {
    return error(parser, ERROR_PARSER_EXPECTED_EXPRESSION);
  }

  ParserResult result = rule->prefix(parser, context);
  if (result.has_error) {
    return result;
  }

  while (1) {
    ParseRule *rule = get_rule(parser->curtok);

    if (precedence > rule->precedence) {
      break;
    }

    ParserResult maybe = rule->infix(parser, context, result.ast);
    if (maybe.has_error) {
      return maybe;
    }

    // when we loop again, the current result becomes the next
    // infix expressions left hand side.
    result = maybe;
  }

  return result;
}

static ParseRule *get_rule(Token token) {
  static ParseRule rules[] = {
      [TOK_END] = {         NULL,  NULL,   PREC_NONE},

      [TOK_ERROR_UNEXPECTED_CHAR]        = {         NULL,  NULL,   PREC_NONE},
      [TOK_ERROR_UNMATCHED_DOUBLE_QUOTE] = {         NULL,  NULL,   PREC_NONE},

      [TOK_BEGIN_PAREN] = {       parens,  NULL,   PREC_NONE},
      [TOK_END_PAREN]   = {         NULL,  NULL,   PREC_NONE},
      [TOK_BEGIN_BRACE] = {         NULL,  NULL,   PREC_NONE},
      [TOK_COMMA]       = {         NULL,  NULL,   PREC_NONE},
      [TOK_DOT]         = {         NULL,  NULL,   PREC_NONE},
      [TOK_SEMICOLON]   = {         NULL,  NULL,   PREC_NONE},
      [TOK_COLON]       = {         NULL,  NULL,   PREC_NONE},
      [TOK_RIGHT_ARROW] = {         NULL,  NULL,   PREC_NONE},

      [TOK_MINUS]         = {         unop, binop,   PREC_TERM},
      [TOK_PLUS]          = {         NULL, binop,   PREC_TERM},
      [TOK_SLASH]         = {         NULL, binop, PREC_FACTOR},
      [TOK_STAR]          = {         NULL, binop, PREC_FACTOR},
      [TOK_PERCENT]       = {         NULL, binop, PREC_FACTOR},
      [TOK_BANG]          = {         NULL,  NULL,   PREC_NONE},
      [TOK_BANG_EQUAL]    = {         NULL,  NULL,   PREC_NONE},
      [TOK_EQUAL]         = {         NULL,  NULL,   PREC_NONE},
      [TOK_EQUAL_EQUAL]   = {         NULL,  NULL,   PREC_NONE},
      [TOK_GREATER]       = {         NULL,  NULL,   PREC_NONE},
      [TOK_GREATER_EQUAL] = {         NULL,  NULL,   PREC_NONE},
      [TOK_LESS]          = {         NULL,  NULL,   PREC_NONE},
      [TOK_LESS_EQUAL]    = {         NULL,  NULL,   PREC_NONE},
      [TOK_AND]           = {         NULL,  NULL,   PREC_NONE},
      [TOK_OR]            = {         NULL,  NULL,   PREC_NONE},
      [TOK_XOR]           = {         NULL,  NULL,   PREC_NONE},

      [TOK_FN]     = {         NULL,  NULL,   PREC_NONE},
      [TOK_VAR]    = {         NULL,  NULL,   PREC_NONE},
      [TOK_CONST]  = {         NULL,  NULL,   PREC_NONE},
      [TOK_RETURN] = {         NULL,  NULL,   PREC_NONE},

      [TOK_NIL]            = {          nil,  NULL,   PREC_NONE},
      [TOK_TRUE]           = { boolean_true,  NULL,   PREC_NONE},
      [TOK_FALSE]          = {boolean_false,  NULL,   PREC_NONE},
      [TOK_INTEGER]        = {      integer,  NULL,   PREC_NONE},
      [TOK_STRING_LITERAL] = {         NULL,  NULL,   PREC_NONE},
      [TOK_IDENTIFIER]     = {         NULL,  NULL,   PREC_NONE},

      [TOK_TYPE_NIL]  = {         NULL,  NULL,   PREC_NONE},
      [TOK_TYPE_BOOL] = {         NULL,  NULL,   PREC_NONE},
      [TOK_TYPE_I64]  = {         NULL,  NULL,   PREC_NONE},
  };

  return &rules[token];
}

i32 parse(char const *restrict buffer, Context *restrict context) {
  assert(buffer != NULL);
  assert(context != NULL);

  Parser parser = parser_create();

  parser_set_view(&parser, buffer);
  nexttok(&parser);

  while (!finished(&parser)) {
    ParserResult maybe = definition(&parser, context);
    if (maybe.has_error) {
      StringView source = context_source_path(context);
      error_print(&maybe.error, source.ptr, curline(&parser));
      parser_result_destroy(&maybe);
      return EXIT_FAILURE;
    }
    
  }

  return EXIT_SUCCESS;
}

i32 parse_source(Context *restrict context) {
  assert(context != NULL);
  String buffer = context_buffer_source(context);
  i32 result    = parse(buffer.buffer, context);
  string_destroy(&buffer);
  return result;
}