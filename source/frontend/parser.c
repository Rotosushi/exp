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

static MaybeError parser_error(Parser *restrict parser, ErrorCode code) {
  return error(code, string_from_view(lexer_current_text(&parser->lexer)));
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

typedef MaybeError (*ParseFunction)(Parser *restrict p, Context *restrict c);

typedef struct ParseRule {
  ParseFunction prefix;
  ParseFunction infix;
  Precedence precedence;
} ParseRule;

/*
  #NOTE: I am going to implement a subset of the total grammar to begin.
  to wit:

  top = declaration ";"

  declaration = "const" identifier = affix

  affix = basic // (binop precedece-parser)?

  basic = integer
        | "true"
        | "false"
        | "nil"
        | string-literal
      //| identifier
      //| unop basic
      //| "(" affix ")"
*/

static Parser parser_create() {
  Parser parser;
  parser.lexer  = lexer_create();
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

static ParseRule *get_rule(Token token);
static MaybeError expression(Parser *restrict parser,
                             Context *restrict context);
static MaybeError parse_precedence(Parser *restrict parser,
                                   Context *restrict context,
                                   Precedence precedence);

static MaybeError constant(Parser *restrict parser, Context *restrict context) {
  nexttok(parser); // eat 'const'

  if (!peek(parser, TOK_IDENTIFIER)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }
  StringView name = context_intern(context, curtxt(parser));
  size_t index =
      context_constants_append(context, value_create_string_literal(name));
  context_emit_push_constant(context, index);

  nexttok(parser);

  if (!expect(parser, TOK_EQUAL)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_EQUAL);
  }

  MaybeError maybe = expression(parser, context);
  if (maybe.has_error) {
    return maybe;
  }

  if (!expect(parser, TOK_SEMICOLON)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
  }

  context_emit_define_global_constant(context);

  return success();
}

typedef struct ErrorOrType {
  bool has_error;
  union {
    Error error;
    Type *type;
  };
} ErrorOrType;

static ErrorOrType parse_scalar_type(Parser *restrict parser,
                                     Context *restrict context) {
  ErrorOrType result;
  switch (parser->curtok) {
  case TOK_NIL_TYPE:
    result.has_error = 0;
    result.type      = context_nil_type(context);
    break;

  case TOK_BOOL_TYPE:
    result.has_error = 0;
    result.type      = context_boolean_type(context);
    break;

  case TOK_INT_TYPE:
    result.has_error = 0;
    result.type      = context_integer_type(context);
    break;

  default:
    result.has_error  = 1;
    result.error.code = ERROR_PARSER_EXPECTED_TYPE;
    string_append_view(&result.error.message, curtxt(parser));
    break;
  }

  nexttok(parser); // eat scalar type

  return result;
}

static ErrorOrType parse_type(Parser *restrict parser,
                              Context *restrict context) {}

typedef struct ErrorOrFormalArgument {
  bool has_error;
  union {
    Error error;
    FormalArgument formal_argument;
  };
} ErrorOrFormalArgument;

static ErrorOrFormalArgument parse_formal_argument(Parser *restrict parser,
                                                   Context *restrict context) {
  ErrorOrFormalArgument result;
  FormalArgument formal_argument;

  if (!peek(parser, TOK_IDENTIFIER)) {
    result.has_error  = 1;
    result.error.code = ERROR_PARSER_EXPECTED_IDENTIFIER;
    string_append_view(&result.error.message, curtxt(parser));
    return result;
  }

  StringView name = context_intern(context, curtxt(parser));

  if (!expect(parser, TOK_COLON)) {
    result.has_error  = 1;
    result.error.code = ERROR_PARSER_EXPECTED_COLON;
    string_append_view(&result.error.message, curtxt(parser));
    return result;
  }

  ErrorOrType maybe = parse_type(parser, context);
  if (maybe.has_error) {
    result.has_error = 1;
    result.error     = maybe.error;
    return result;
  }

  result.has_error            = 0;
  result.formal_argument.name = name;
  result.formal_argument.type = maybe.type;
  return result;
}

typedef struct ErrorOrFormalArgumentList {
  bool has_error;
  union {
    Error error;
    FormalArgumentList formal_argument_list;
  };
} ErrorOrFormalArgumentList;

static ErrorOrFormalArgumentList
parse_formal_argument_list(Parser *restrict parser, Context *restrict context) {
  FormalArgumentList fal = formal_argument_list_create();
}

static MaybeError function(Parser *restrict parser, Context *restrict context) {
  nexttok(parser); // eat 'fn'

  // parse function arguments
  //  -> function arguments are represented with an array of
  //      identifier, type pairs
  // parse return type
  // parse function body
  //  -> the body is represented with a new chunk of Bytecode
  //      this means that the parser needs to emit bytecode
  //      into this new chunk when parsing the body.
  // create a new function constant
  // add bytecode to add function to
  //  the symbol table

  return success();
}

static MaybeError parens(Parser *restrict parser, Context *restrict context) {
  nexttok(parser); // eat '('

  MaybeError maybe = expression(parser, context);
  if (maybe.has_error) {
    return maybe;
  }

  if (!expect(parser, TOK_END_PAREN)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_END_PAREN);
  }

  return success();
}

static MaybeError unop(Parser *restrict parser, Context *restrict context) {
  Token op = parser->curtok;
  nexttok(parser);

  parse_precedence(parser, context, PREC_UNARY);

  switch (op) {
  case TOK_MINUS:
    context_emit_unop_minus(context);
    break;

  default:
    unreachable();
  }

  return success();
}

static MaybeError binop(Parser *restrict parser, Context *restrict context) {
  Token op        = parser->curtok;
  ParseRule *rule = get_rule(op);
  nexttok(parser); // eat the operator

  parse_precedence(parser, context, (Precedence)(rule->precedence + 1));

  switch (op) {
  case TOK_PLUS:
    context_emit_binop_plus(context);
    break;

  case TOK_MINUS:
    context_emit_binop_minus(context);
    break;

  case TOK_STAR:
    context_emit_binop_star(context);
    break;

  case TOK_SLASH:
    context_emit_binop_slash(context);
    break;

  case TOK_PERCENT:
    context_emit_binop_percent(context);
    break;

  default:
    unreachable();
  }

  return success();
}

static MaybeError return_(Parser *restrict parser, Context *restrict context) {
  nexttok(parser); // eat "return"

  MaybeError maybe = expression(parser, context);
  if (maybe.has_error) {
    return maybe;
  }

  if (!expect(parser, TOK_SEMICOLON)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
  }

  context_emit_return(context);

  return success();
}

static MaybeError nil(Parser *restrict parser, Context *restrict context) {
  size_t index = context_constants_append(context, value_create_nil());
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError boolean_true(Parser *restrict parser,
                               Context *restrict context) {
  size_t index = context_constants_append(context, value_create_boolean(1));
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError boolean_false(Parser *restrict parser,
                                Context *restrict context) {
  size_t index = context_constants_append(context, value_create_boolean(0));
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError integer(Parser *restrict parser, Context *restrict context) {
  StringView sv = curtxt(parser);
  long integer  = strtol(sv.ptr, NULL, 10);
  if (errno == ERANGE) {
    return parser_error(parser, ERROR_PARSER_INTEGER_TO_LARGE);
  }

  size_t index =
      context_constants_append(context, value_create_integer(integer));
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError string_literal(Parser *restrict parser,
                                 Context *restrict context) {
  // #NOTE: when the lexer scans a string literal, the trailing '"'
  // is left in the curtxt. so we 'consume' that here.
  // this feels hacky, because the lexer isn't passing back
  // the 'correct' text associated with the token.
  StringView sv = curtxt(parser);
  sv.length -= 1;
  StringView sl = context_intern(context, sv);
  size_t index =
      context_constants_append(context, value_create_string_literal(sl));
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError expression(Parser *restrict parser,
                             Context *restrict context) {
  return parse_precedence(parser, context, PREC_ASSIGNMENT);
}

static MaybeError parse_precedence(Parser *restrict parser,
                                   Context *restrict context,
                                   Precedence precedence) {
  ParseFunction prefix = get_rule(parser->curtok)->prefix;
  if (prefix == NULL) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_EXPRESSION);
  }

  MaybeError maybe = prefix(parser, context);
  if (maybe.has_error) {
    return maybe;
  }

  while (1) {
    ParseRule *rule = get_rule(parser->curtok);

    if (precedence > rule->precedence) {
      break;
    }

    MaybeError maybe = rule->infix(parser, context);
    if (maybe.has_error) {
      return maybe;
    }
  }

  return success();
}

static ParseRule *get_rule(Token token) {
  static ParseRule rules[] = {
      [TOK_END] = {          NULL,  NULL,   PREC_NONE},

      [TOK_ERROR_UNEXPECTED_CHAR]        = {          NULL,  NULL,   PREC_NONE},
      [TOK_ERROR_UNMATCHED_DOUBLE_QUOTE] = {          NULL,  NULL,   PREC_NONE},

      [TOK_BEGIN_PAREN] = {        parens,  NULL,   PREC_NONE},
      [TOK_END_PAREN]   = {          NULL,  NULL,   PREC_NONE},
      [TOK_BEGIN_BRACE] = {          NULL,  NULL,   PREC_NONE},
      [TOK_COMMA]       = {          NULL,  NULL,   PREC_NONE},
      [TOK_DOT]         = {          NULL,  NULL,   PREC_NONE},
      [TOK_SEMICOLON]   = {          NULL,  NULL,   PREC_NONE},
      [TOK_COLON]       = {          NULL,  NULL,   PREC_NONE},
      [TOK_RIGHT_ARROW] = {          NULL,  NULL,   PREC_NONE},

      [TOK_MINUS]         = {          unop, binop,   PREC_TERM},
      [TOK_PLUS]          = {          NULL, binop,   PREC_TERM},
      [TOK_SLASH]         = {          NULL, binop, PREC_FACTOR},
      [TOK_STAR]          = {          NULL, binop, PREC_FACTOR},
      [TOK_PERCENT]       = {          NULL, binop, PREC_FACTOR},
      [TOK_BANG]          = {          NULL,  NULL,   PREC_NONE},
      [TOK_BANG_EQUAL]    = {          NULL,  NULL,   PREC_NONE},
      [TOK_EQUAL]         = {          NULL,  NULL,   PREC_NONE},
      [TOK_EQUAL_EQUAL]   = {          NULL,  NULL,   PREC_NONE},
      [TOK_GREATER]       = {          NULL,  NULL,   PREC_NONE},
      [TOK_GREATER_EQUAL] = {          NULL,  NULL,   PREC_NONE},
      [TOK_LESS]          = {          NULL,  NULL,   PREC_NONE},
      [TOK_LESS_EQUAL]    = {          NULL,  NULL,   PREC_NONE},
      [TOK_AND]           = {          NULL,  NULL,   PREC_NONE},
      [TOK_OR]            = {          NULL,  NULL,   PREC_NONE},
      [TOK_XOR]           = {          NULL,  NULL,   PREC_NONE},

      [TOK_FN]     = {      function,  NULL,   PREC_NONE},
      [TOK_VAR]    = {          NULL,  NULL,   PREC_NONE},
      [TOK_CONST]  = {      constant,  NULL,   PREC_NONE},
      [TOK_RETURN] = {       return_,  NULL,   PREC_NONE},

      [TOK_NIL]            = {           nil,  NULL,   PREC_NONE},
      [TOK_TRUE]           = {  boolean_true,  NULL,   PREC_NONE},
      [TOK_FALSE]          = { boolean_false,  NULL,   PREC_NONE},
      [TOK_INTEGER]        = {       integer,  NULL,   PREC_NONE},
      [TOK_STRING_LITERAL] = {string_literal,  NULL,   PREC_NONE},
      [TOK_IDENTIFIER]     = {          NULL,  NULL,   PREC_NONE},

      [TOK_NIL_TYPE]            = {          NULL,  NULL,   PREC_NONE},
      [TOK_BOOL_TYPE]           = {          NULL,  NULL,   PREC_NONE},
      [TOK_INT_TYPE]            = {          NULL,  NULL,   PREC_NONE},
      [TOK_STRING_LITERAL_TYPE] = {          NULL,  NULL,   PREC_NONE},
  };

  return &rules[token];
}

int parse(char const *restrict buffer, Context *restrict context) {
  assert(buffer != NULL);
  assert(context != NULL);

  Parser parser = parser_create();

  parser_set_view(&parser, buffer);
  nexttok(&parser);

  while (!finished(&parser)) {
    MaybeError maybe = expression(&parser, context);
    if (maybe.has_error) {
      StringView source = context_source_path(context);
      error_print(&maybe.error, source.ptr, curline(&parser));
      return EXIT_FAILURE;
    }
    maybe_error_destroy(&maybe);
  }

  context_emit_stop(context);

  return EXIT_SUCCESS;
}

int parse_source(Context *restrict context) {
  assert(context != NULL);
  String buffer = context_buffer_source(context);
  int result    = parse(buffer.buffer, context);
  string_destroy(&buffer);
  return result;
}