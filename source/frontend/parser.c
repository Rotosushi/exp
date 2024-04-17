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
  u64 index =
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

static MaybeError parse_scalar_type(Parser *restrict parser,
                                    Context *restrict context, Type **type) {
  switch (parser->curtok) {
  case TOK_NIL_TYPE:
    *type = context_nil_type(context);
    break;

  case TOK_BOOL_TYPE:
    *type = context_boolean_type(context);
    break;

  case TOK_INT_TYPE:
    *type = context_integer_type(context);
    break;

  default:
    return parser_error(parser, ERROR_PARSER_EXPECTED_TYPE);
  }

  nexttok(parser); // eat scalar type
  return success();
}

static MaybeError parse_type(Parser *restrict parser, Context *restrict context,
                             Type **type) {
  if (expect(parser, TOK_FN)) {
    ArgumentTypes argument_types = argument_types_create();
    // #TODO: parens could be used to predict a tuple-type
    // which could be parsed as a scalar type, which could
    // then make parens optional for an argument list of
    // length one
    if (!expect(parser, TOK_BEGIN_PAREN)) {
      return parser_error(parser, ERROR_PARSER_EXPECTED_BEGIN_PAREN);
    }

    do {
      Type *arg        = NULL;
      MaybeError maybe = parse_scalar_type(parser, context, &arg);
      if (maybe.has_error) {
        argument_types_destroy(&argument_types);
        return maybe;
      }
      assert(arg != NULL);

      argument_types_append(&argument_types, arg);
    } while (expect(parser, TOK_COMMA));

    if (!expect(parser, TOK_END_PAREN)) {
      argument_types_destroy(&argument_types);
      return parser_error(parser, ERROR_PARSER_EXPECTED_END_PAREN);
    }

    if (!expect(parser, TOK_RIGHT_ARROW)) {
      argument_types_destroy(&argument_types);
      return parser_error(parser, ERROR_PARSER_EXPECTED_RIGHT_ARROW);
    }
    Type *return_type = NULL;
    MaybeError maybe  = parse_scalar_type(parser, context, &return_type);
    if (maybe.has_error) {
      argument_types_destroy(&argument_types);

      return maybe;
    }
    assert(return_type != NULL);

    *type = context_function_type(context, return_type, argument_types);
    return success();
  } else {
    return parse_scalar_type(parser, context, type);
  }
}

static MaybeError parse_formal_argument(Parser *restrict parser,
                                        Context *restrict context,
                                        FormalArgument *formal_argument) {
  if (!peek(parser, TOK_IDENTIFIER)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }

  StringView name = context_intern(context, curtxt(parser));

  if (!expect(parser, TOK_COLON)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_COLON);
  }

  Type *type       = NULL;
  MaybeError maybe = parse_type(parser, context, &type);
  if (maybe.has_error) {
    return maybe;
  }
  assert(type != NULL);

  *formal_argument = (FormalArgument){name, type};
  return success();
}

static MaybeError parse_formal_argument_list(Parser *restrict parser,
                                             Context *restrict context,
                                             FormalArgumentList *fal) {
  if (!expect(parser, TOK_BEGIN_PAREN)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_BEGIN_PAREN);
  }

  if (!expect(parser, TOK_END_PAREN)) {
    do {
      FormalArgument arg;
      MaybeError maybe = parse_formal_argument(parser, context, &arg);
      if (maybe.has_error) {
        return maybe;
      }

      formal_argument_list_append(fal, arg.name, arg.type);
    } while (expect(parser, TOK_COMMA));

    if (!expect(parser, TOK_END_PAREN)) {
      return parser_error(parser, ERROR_PARSER_EXPECTED_END_PAREN);
    }
  }

  return success();
}

static MaybeError function(Parser *restrict parser, Context *restrict context) {
  nexttok(parser); // eat 'fn'

  if (!peek(parser, TOK_IDENTIFIER)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }

  StringView name = context_intern(context, curtxt(parser));

  nexttok(parser);

  Value value        = value_create_function(name);
  Function *function = &value.function;
  Bytecode *previous_bytecode =
      context_current_function_body(context, &function->body);

  // parse function arguments
  {
    MaybeError maybe =
        parse_formal_argument_list(parser, context, &function->arguments);
    if (maybe.has_error) {
      value_destroy(&value);
      return maybe;
    }
  }

  // parse return type
  if (!expect(parser, TOK_RIGHT_ARROW)) {
    return parser_error(parser, ERROR_PARSER_EXPECTED_RIGHT_ARROW);
  }

  {
    MaybeError maybe = parse_type(parser, context, &function->return_type);
    if (maybe.has_error) {
      value_destroy(&value);

      return maybe;
    }
  }

  // parse function body
  if (!expect(parser, TOK_BEGIN_BRACE)) {
    value_destroy(&value);
    return parser_error(parser, ERROR_PARSER_EXPECTED_BEGIN_BRACE);
  }

  // #NOTE: due to the above call of "context_current_function_body"
  // these calls to "expression" will emit bytecode into the
  // body of the current function.
  while (!expect(parser, TOK_END_BRACE)) {
    MaybeError maybe = expression(parser, context);
    if (maybe.has_error) {
      value_destroy(&value);
      context_current_function_body(context, previous_bytecode);
      return maybe;
    }
  }

  context_current_function_body(context, previous_bytecode);

  u64 index = context_constants_append(context, value);
  context_emit_push_constant(context, index);
  context_emit_define_global_constant(context);
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
  u64 index = context_constants_append(context, value_create_nil());
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError boolean_true(Parser *restrict parser,
                               Context *restrict context) {
  u64 index = context_constants_append(context, value_create_boolean(1));
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError boolean_false(Parser *restrict parser,
                                Context *restrict context) {
  u64 index = context_constants_append(context, value_create_boolean(0));
  context_emit_push_constant(context, index);
  nexttok(parser);
  return success();
}

static MaybeError integer(Parser *restrict parser, Context *restrict context) {
  StringView sv = curtxt(parser);
  i64 integer   = strtol(sv.ptr, NULL, 10);
  if (errno == ERANGE) {
    return parser_error(parser, ERROR_PARSER_INTEGER_TO_LARGE);
  }

  u64 index = context_constants_append(context, value_create_integer(integer));
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
  u64 index =
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

i32 parse(char const *restrict buffer, Context *restrict context) {
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

i32 parse_source(Context *restrict context) {
  assert(context != NULL);
  String buffer = context_buffer_source(context);
  i32 result    = parse(buffer.buffer, context);
  string_destroy(&buffer);
  return result;
}