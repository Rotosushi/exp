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
#include "utility/config.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

typedef struct Parser {
  Lexer lexer;
  Token curtok;
} Parser;

typedef struct ParserResult {
  bool has_error;
  union {
    Operand result;
    Error error;
  };
} ParserResult;

static void parser_result_destroy(ParserResult *restrict pr) {
  if (pr->has_error) { error_destroy(&pr->error); }
}

static ParserResult error(Parser *restrict p, ErrorCode code) {
  ParserResult result;
  result.has_error = 1;
  result.error     = error_construct(code, lexer_current_text(&p->lexer));
  return result;
}

static ParserResult success(Operand result) {
  ParserResult pr;
  pr.has_error = 0;
  pr.result    = result;
  return pr;
}

static ParserResult from_fold(FoldResult fold) {
  ParserResult result = {.has_error = fold.has_error};
  if (fold.has_error) {
    result.error = fold.error;
  } else {
    result.result = fold.operand;
  }
  return result;
}

static Operand zero() {
  Operand o = {.format = OPRFMT_IMMEDIATE, .common = 0};
  return o;
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
typedef ParserResult (*InfixFunction)(Parser *restrict p,
                                      Context *restrict c,
                                      Operand left);

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

static void
parser_set_view(Parser *restrict parser, char const *buffer, u64 length) {
  assert(parser != NULL);
  assert(buffer != NULL);
  lexer_set_view(&(parser->lexer), buffer, length);
}

static bool finished(Parser *restrict parser) {
  return parser->curtok == TOK_END;
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
static ParserResult expression(Parser *restrict p, Context *restrict c);
static ParserResult parse_precedence(Parser *restrict p,
                                     Context *restrict c,
                                     Precedence precedence);

static ParserResult
parse_scalar_type(Parser *restrict p, Context *restrict c, Type **type) {
  switch (p->curtok) {
  case TOK_TYPE_NIL: *type = context_nil_type(c); break;

  case TOK_TYPE_BOOL: *type = context_boolean_type(c); break;

  case TOK_TYPE_I64: *type = context_i64_type(c); break;

  default: return error(p, ERROR_PARSER_EXPECTED_TYPE);
  }

  nexttok(p); // eat <scalar-type>
  return success(zero());
}

static ParserResult
parse_type(Parser *restrict p, Context *restrict c, Type **type) {
  // #TODO: handle composite types
  return parse_scalar_type(p, c, type);
}

static ParserResult parse_formal_argument(Parser *restrict p,
                                          Context *restrict c,
                                          FormalArgument *arg) {
  if (!peek(p, TOK_IDENTIFIER)) {
    return error(p, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }

  StringView name = context_intern(c, curtxt(p));
  nexttok(p);

  if (!expect(p, TOK_COLON)) { return error(p, ERROR_PARSER_EXPECTED_COLON); }

  Type *type         = NULL;
  ParserResult maybe = parse_type(p, c, &type);
  if (maybe.has_error) { return maybe; }
  assert(type != NULL);

  arg->name = name;
  arg->type = type;
  return success(zero());
}

static ParserResult parse_formal_argument_list(Parser *restrict p,
                                               Context *restrict c,
                                               FunctionBody *body) {
  // we have to add arguments to the local frame,
  // pretty sure we can just
  // reserve the first n locals for the first n arguments.
  // The issue is how do we associate identifiers
  // with local slots. such that identifiers can be
  // used which will refer to the correct local slot.
  // And this solution will probably
  // carry over to how local definitions are handled.
  // for now we don't support function arguments.
  FormalArgumentList *args = &body->arguments;
  // #note: the nil literal is spelled "()", which is
  // lexically identical to an empty argument list
  if (expect(p, TOK_NIL)) { return success(zero()); }

  if (!expect(p, TOK_BEGIN_PAREN)) {
    return error(p, ERROR_PARSER_EXPECTED_BEGIN_PAREN);
  }

  if (!expect(p, TOK_END_PAREN)) {
    do {
      FormalArgument arg;

      ParserResult maybe = parse_formal_argument(p, c, &arg);
      if (maybe.has_error) { return maybe; }

      formal_argument_list_append(args, arg);
    } while (!expect(p, TOK_END_PAREN));
  }

  return success(zero());
}

// "const" identifier "=" <expression> ";"
static ParserResult constant(Parser *restrict p, Context *restrict c) {
  nexttok(p); // eat 'const'

  if (!peek(p, TOK_IDENTIFIER)) {
    return error(p, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }
  StringView name = context_intern(c, curtxt(p));
  nexttok(p);

  if (!expect(p, TOK_EQUAL)) { return error(p, ERROR_PARSER_EXPECTED_EQUAL); }

  ParserResult maybe = expression(p, c);
  if (maybe.has_error) { return maybe; }

  if (!expect(p, TOK_SEMICOLON)) {
    return error(p, ERROR_PARSER_EXPECTED_SEMICOLON);
  }

  context_def_const(c, name, maybe.result);
  return success(zero());
}

// "return" <expression> ";"
static ParserResult return_(Parser *restrict p, Context *restrict c) {
  nexttok(p); // eat "return"

  ParserResult maybe = expression(p, c);
  if (maybe.has_error) { return maybe; }

  if (!expect(p, TOK_SEMICOLON)) {
    return error(p, ERROR_PARSER_EXPECTED_SEMICOLON);
  }

  context_emit_return(c, maybe.result);
  return success(zero());
}

static ParserResult statement(Parser *restrict p, Context *restrict c) {
  switch (p->curtok) {
  case TOK_RETURN: return return_(p, c);

  case TOK_CONST: return constant(p, c);

  default: {
    ParserResult result = expression(p, c);
    if (result.has_error) { return result; }

    if (!expect(p, TOK_SEMICOLON)) {
      return error(p, ERROR_PARSER_EXPECTED_SEMICOLON);
    }

    return result;
  }
  }
}

static ParserResult parse_block(Parser *restrict p, Context *restrict c) {
  if (!expect(p, TOK_BEGIN_BRACE)) {
    return error(p, ERROR_PARSER_EXPECTED_BEGIN_BRACE);
  }

  while (!expect(p, TOK_END_BRACE)) {
    ParserResult maybe = statement(p, c);
    if (maybe.has_error) { return maybe; }
  }

  return success(zero());
}

static ParserResult function(Parser *restrict p, Context *restrict c) {
  nexttok(p); // eat "fn"

  if (!peek(p, TOK_IDENTIFIER)) {
    return error(p, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }

  StringView name = context_intern(c, curtxt(p));
  nexttok(p);

  FunctionBody *body = context_enter_function(c, name);

  {
    ParserResult maybe = parse_formal_argument_list(p, c, body);
    if (maybe.has_error) { return maybe; }
  }

  {
    ParserResult maybe = parse_block(p, c);
    if (maybe.has_error) { return maybe; }
  }

  context_leave_function(c);

  // #if EXP_DEBUG
  //   file_write("parsed a function: \n fn ", stdout);
  //   print_string_view(name, stdout);
  //   print_function_body(body, stdout);
  //   file_write("\n", stdout);
  // #endif

  return success(zero());
}

static ParserResult definition(Parser *restrict p, Context *restrict c) {
  switch (p->curtok) {
  case TOK_FN: return function(p, c);

  default: return error(p, ERROR_PARSER_EXPECTED_KEYWORD_FN);
  }
}

static ParserResult parens(Parser *restrict p, Context *restrict c) {
  nexttok(p); // eat '('

  ParserResult result = expression(p, c);
  if (result.has_error) { return result; }

  if (!expect(p, TOK_END_PAREN)) {
    return error(p, ERROR_PARSER_EXPECTED_END_PAREN);
  }

  return result;
}

static ParserResult unop(Parser *restrict p, Context *restrict c) {
  Token op = p->curtok;
  nexttok(p);

  ParserResult maybe = parse_precedence(p, c, PREC_UNARY);
  if (maybe.has_error) { return maybe; }

  switch (op) {
  case TOK_MINUS: return from_fold(context_emit_neg(c, maybe.result));

  default: unreachable();
  }
}

static ParserResult
binop(Parser *restrict p, Context *restrict c, Operand left) {
  Token op        = p->curtok;
  ParseRule *rule = get_rule(op);
  nexttok(p); // eat the operator

  ParserResult maybe =
      parse_precedence(p, c, (Precedence)(rule->precedence + 1));
  if (maybe.has_error) { return maybe; }
  Operand right = maybe.result;

  switch (op) {
  case TOK_PLUS:    return from_fold(context_emit_add(c, left, right));
  case TOK_MINUS:   return from_fold(context_emit_sub(c, left, right));
  case TOK_STAR:    return from_fold(context_emit_mul(c, left, right));
  case TOK_SLASH:   return from_fold(context_emit_div(c, left, right));
  case TOK_PERCENT: return from_fold(context_emit_mod(c, left, right));

  default: unreachable();
  }
}

static ParserResult nil(Parser *restrict p, Context *restrict c) {
  nexttok(p);
  Operand idx = context_constants_append(c, value_create_nil());
  return success(idx);
}

static ParserResult boolean_true(Parser *restrict p, Context *restrict c) {
  nexttok(p);
  Operand idx = context_constants_append(c, value_create_boolean(1));
  return success(idx);
}

static ParserResult boolean_false(Parser *restrict p, Context *restrict c) {
  nexttok(p);
  Operand idx = context_constants_append(c, value_create_boolean(0));
  return success(idx);
}

static ParserResult integer(Parser *restrict p, Context *restrict c) {
  StringView sv = curtxt(p);
  i64 integer   = str_to_i64(sv.ptr, sv.length, RADIX_DECIMAL);

  nexttok(p);
  Operand B;
  if ((integer >= 0) && (integer < u16_MAX)) {
    B = opr_immediate((u16)integer);
  } else {
    B = context_constants_append(c, value_create_i64(integer));
  }

  return success(B);
}

static ParserResult identifier(Parser *restrict p, Context *restrict c) {
  StringView name = context_intern(c, curtxt(p));
  nexttok(p);
  LocalVariable *var = context_lookup_local(c, name);
  if (var == NULL) { return error(p, ERROR_TYPECHECK_UNDEFINED_SYMBOL); }

  return success(opr_ssa(var->ssa));
}

static ParserResult expression(Parser *restrict p, Context *restrict c) {
  return parse_precedence(p, c, PREC_ASSIGNMENT);
}

static ParserResult parse_precedence(Parser *restrict p,
                                     Context *restrict c,
                                     Precedence precedence) {
  ParseRule *rule = get_rule(p->curtok);
  if (rule->prefix == NULL) {
    return error(p, ERROR_PARSER_EXPECTED_EXPRESSION);
  }

  ParserResult result = rule->prefix(p, c);
  if (result.has_error) { return result; }

  while (1) {
    ParseRule *rule = get_rule(p->curtok);

    if (precedence > rule->precedence) { break; }

    ParserResult maybe = rule->infix(p, c, result.result);
    if (maybe.has_error) { return maybe; }

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
      [TOK_IDENTIFIER]     = {   identifier,  NULL,   PREC_NONE},

      [TOK_TYPE_NIL]  = {         NULL,  NULL,   PREC_NONE},
      [TOK_TYPE_BOOL] = {         NULL,  NULL,   PREC_NONE},
      [TOK_TYPE_I64]  = {         NULL,  NULL,   PREC_NONE},
  };

  return &rules[token];
}

i32 parse_buffer(char const *restrict buffer, u64 length, Context *restrict c) {
  assert(buffer != NULL);
  assert(c != NULL);

  Parser p = parser_create();

  parser_set_view(&p, buffer, length);
  nexttok(&p);

  while (!finished(&p)) {
    ParserResult maybe = definition(&p, c);
    if (maybe.has_error) {
      error_print(&maybe.error, context_source_path(c), curline(&p));
      parser_result_destroy(&maybe);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

i32 parse_source(Context *restrict c) {
  assert(c != NULL);
  StringView path = context_source_path(c);
  FILE *file      = file_open(path.ptr, "r");
  String buffer   = string_from_file(file);
  file_close(file);
  i32 result = parse_buffer(string_to_cstring(&buffer), buffer.length, c);
  string_destroy(&buffer);
  return result;
}