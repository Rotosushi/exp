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
#include <stdio.h>
#include <stdlib.h>

#include "env/error.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "imr/operand.h"
#include "utility/numeric_conversions.h"
#include "utility/unreachable.h"

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
  ParserResult result = {
      .has_error = 1,
      .error     = error_construct(code, lexer_current_text(&p->lexer))};
  return result;
}

static ParserResult success(Operand result) {
  ParserResult pr = {.has_error = 0, .result = result};
  return pr;
}

static Operand zero() { return operand_immediate(0); }

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

static bool peek(Parser *restrict parser, Token token) {
  return parser->curtok == token;
}

static ParserResult comment(Parser *restrict parser) {
  // a comment starts with '/*' and lasts until
  // it's matching '*/'.
  // we handle any number of comment blocks to
  // appear sequentially. and if a comment
  // block begins within a comment block that starts
  // a new comment block, meaning that two '*/' are
  // needed to leave the 'comment' state.
  // # TODO do we want to do anything special with markup here?
  while (parser->curtok != TOK_END_COMMENT) {
    parser->curtok = lexer_scan(&parser->lexer);

    if (peek(parser, TOK_END)) {
      return error(parser, ERROR_PARSER_EXPECTED_END_COMMENT);
    }

    if (peek(parser, TOK_BEGIN_COMMENT)) {
      ParserResult result = comment(parser);
      if (result.has_error) { return result; }
    }
  }

  // eat '*/'
  parser->curtok = lexer_scan(&parser->lexer);
  return success(zero());
}

static ParserResult nexttok(Parser *restrict parser) {
  if (lexer_at_end(&parser->lexer)) {
    parser->curtok = TOK_END;
    return success(zero());
  }

  parser->curtok = lexer_scan(&parser->lexer);

  while (parser->curtok == TOK_BEGIN_COMMENT) {
    ParserResult result = comment(parser);
    if (result.has_error) { return result; }
  }

  return success(zero());
}

typedef struct ExpectResult {
  bool has_error;
  union {
    bool found;
    Error error;
  };
} ExpectResult;

static ExpectResult expect_error(ParserResult result) {
  ExpectResult expect = {.has_error = result.has_error, .error = result.error};
  return expect;
}

static ExpectResult expect_success(bool found) {
  ExpectResult expect = {.has_error = false, .found = found};
  return expect;
}

static ParserResult from_expect(ExpectResult expect) {
  ParserResult result = {.has_error = expect.has_error};
  if (expect.has_error) {
    result.error = expect.error;
  } else {
    result.result = zero();
  }
  return result;
}

static ExpectResult expect(Parser *restrict parser, Token token) {
  if (peek(parser, token)) {
    ParserResult result = nexttok(parser);
    if (result.has_error) {
      return expect_error(result);
    } else {
      return expect_success(true);
    }
  } else {
    return expect_success(false);
  }
}

#define EXPECT(name, token)                                                    \
  ExpectResult name = expect(p, token);                                        \
  if (name.has_error) { return from_expect(name); }

#define NEXTTOK(parser)                                                        \
  {                                                                            \
    ParserResult result = nexttok(parser);                                     \
    if (result.has_error) { return result; }                                   \
  }

#define TRY(name, call)                                                        \
  ParserResult name = call;                                                    \
  if (name.has_error) { return name; }

static ParseRule *get_rule(Token token);
static ParserResult expression(Parser *restrict p, Context *restrict c);
static ParserResult parse_precedence(Parser *restrict p,
                                     Context *restrict c,
                                     Precedence precedence);

static ParserResult
parse_type(Parser *restrict p, Context *restrict c, Type **type);

static ParserResult
parse_tuple_type(Parser *restrict p, Context *restrict c, Type **type) {
  NEXTTOK(p); // eat '('

  // an empty tuple type is equivalent to a nil type.
  EXPECT(nil, TOK_NIL);
  if (nil.found) {
    *type = context_nil_type(c);
    return success(zero());
  }

  TupleType tuple_type = tuple_type_create();

  bool found_comma = false;
  do {
    Type *element = NULL;
    TRY(result, parse_type(p, c, &element));
    assert(element != NULL);

    tuple_type_append(&tuple_type, element);

    EXPECT(comma, TOK_COMMA);
    found_comma = comma.found;
  } while (found_comma);

  EXPECT(end_paren, TOK_END_PAREN);
  if (!end_paren.found) { return error(p, ERROR_PARSER_EXPECTED_END_PAREN); }

  // a tuple type of length 1 is equivalent to that type.
  if (tuple_type.size == 1) {
    *type = tuple_type.types[0];
    tuple_type_destroy(&tuple_type);
  } else {
    *type = context_tuple_type(c, tuple_type);
  }

  return success(zero());
}

static ParserResult
parse_type(Parser *restrict p, Context *restrict c, Type **type) {
  switch (p->curtok) {
  // composite types
  case TOK_BEGIN_PAREN: return parse_tuple_type(p, c, type);

  // scalar types
  case TOK_NIL:       *type = context_nil_type(c); break;
  case TOK_TYPE_NIL:  *type = context_nil_type(c); break;
  case TOK_TYPE_BOOL: *type = context_boolean_type(c); break;
  case TOK_TYPE_I64:  *type = context_i64_type(c); break;

  default: return error(p, ERROR_PARSER_EXPECTED_TYPE);
  }

  NEXTTOK(p); // eat scalar-type
  return success(zero());
}

// formal-argument = identifier ":" type
static ParserResult parse_formal_argument(Parser *restrict p,
                                          Context *restrict c,
                                          FormalArgument *arg) {
  if (!peek(p, TOK_IDENTIFIER)) {
    return error(p, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }

  StringView name = context_intern(c, curtxt(p));
  NEXTTOK(p);

  EXPECT(colon, TOK_COLON);
  if (!colon.found) { return error(p, ERROR_PARSER_EXPECTED_COLON); }

  Type *type = NULL;
  TRY(maybe, parse_type(p, c, &type));
  assert(type != NULL);

  arg->name = name;
  arg->type = type;
  return success(zero());
}

// formal-argument-list = "(" (formal-argument ("," formal-argument)*)? ")"
static ParserResult parse_formal_argument_list(Parser *restrict p,
                                               Context *restrict c,
                                               FunctionBody *body) {
  // #note: the nil literal is spelled "()", which is
  // lexically identical to an empty argument list. so we parse it as such
  EXPECT(nil, TOK_NIL);
  if (nil.found) { return success(zero()); }

  EXPECT(begin_paren, TOK_BEGIN_PAREN);
  if (!begin_paren.found) {
    return error(p, ERROR_PARSER_EXPECTED_BEGIN_PAREN);
  }

  EXPECT(end_paren, TOK_END_PAREN);
  if (!end_paren.found) {
    u8 index         = 0;
    bool comma_found = false;
    do {
      FormalArgument arg = {.index = index++};

      TRY(maybe, parse_formal_argument(p, c, &arg));

      function_body_new_argument(body, arg);

      EXPECT(comma, TOK_COMMA);
      comma_found = comma.found;
    } while (comma_found);

    EXPECT(end_paren, TOK_END_PAREN);
    if (!end_paren.found) { return error(p, ERROR_PARSER_EXPECTED_END_PAREN); }
  }

  return success(zero());
}

// return = "return" expression ";"
static ParserResult return_(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat "return"

  TRY(maybe, expression(p, c));

  EXPECT(semicolon, TOK_SEMICOLON);
  if (!semicolon.found) { return error(p, ERROR_PARSER_EXPECTED_SEMICOLON); }

  context_emit_return(c, maybe.result);
  return success(zero());
}

// constant = "const" identifier "=" expression ";"
static ParserResult constant(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat 'const'

  if (!peek(p, TOK_IDENTIFIER)) {
    return error(p, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }
  StringView name = context_intern(c, curtxt(p));
  NEXTTOK(p);

  EXPECT(equal, TOK_EQUAL);
  if (!equal.found) { return error(p, ERROR_PARSER_EXPECTED_EQUAL); }

  TRY(maybe, expression(p, c));

  EXPECT(semicolon, TOK_SEMICOLON);
  if (!semicolon.found) { return error(p, ERROR_PARSER_EXPECTED_SEMICOLON); }

  context_def_local_const(c, name, maybe.result);
  return success(zero());
}

// statement = return
//           | constant
//           | expression
static ParserResult statement(Parser *restrict p, Context *restrict c) {
  switch (p->curtok) {
  case TOK_RETURN: return return_(p, c);
  case TOK_CONST:  return constant(p, c);

  default: {
    TRY(result, expression(p, c));

    EXPECT(semicolon, TOK_SEMICOLON);
    if (!semicolon.found) { return error(p, ERROR_PARSER_EXPECTED_SEMICOLON); }

    return result;
  }
  }
}

static ParserResult parse_block(Parser *restrict p, Context *restrict c) {
  EXPECT(begin_brace, TOK_BEGIN_BRACE);
  if (!begin_brace.found) {
    return error(p, ERROR_PARSER_EXPECTED_BEGIN_BRACE);
  }

  EXPECT(end_brace, TOK_END_BRACE);
  bool found_end_brace = end_brace.found;
  while (!found_end_brace) {
    TRY(maybe, statement(p, c));
    EXPECT(end_brace, TOK_END_BRACE);
    found_end_brace = end_brace.found;
  }

  return success(zero());
}

static ParserResult function(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat "fn"

  if (!peek(p, TOK_IDENTIFIER)) {
    return error(p, ERROR_PARSER_EXPECTED_IDENTIFIER);
  }

  StringView name = context_intern(c, curtxt(p));
  NEXTTOK(p);

  FunctionBody *body = context_enter_function(c, name);

  { TRY(maybe, parse_formal_argument_list(p, c, body)); }

  EXPECT(right_arrow, TOK_RIGHT_ARROW);
  if (right_arrow.found) { TRY(maybe, parse_type(p, c, &body->return_type)); }

  { TRY(maybe, parse_block(p, c)); }

#if EXP_DEBUG
  file_write("parsed a function: \nfn ", stdout);
  print_string_view(name, stdout);
  print_function_body(body, stdout, c);
  file_write("\n", stdout);
#endif

  context_leave_function(c);
  return success(zero());
}

static ParserResult definition(Parser *restrict p, Context *restrict c) {
  switch (p->curtok) {
  case TOK_FN: return function(p, c);

  default: return error(p, ERROR_PARSER_EXPECTED_KEYWORD_FN);
  }
}

static ParserResult parens(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat '('

  Tuple tuple      = tuple_create();
  bool found_comma = false;
  do {
    TRY(result, expression(p, c));
    tuple_append(&tuple, result.result);

    EXPECT(comma, TOK_COMMA);
    found_comma = comma.found;
  } while (found_comma);

  EXPECT(end_paren, TOK_END_PAREN);
  if (!end_paren.found) { return error(p, ERROR_PARSER_EXPECTED_END_PAREN); }

  Operand result;

  if (tuple.size == 0) {
    result = context_values_append(c, value_create_nil());
    tuple_destroy(&tuple);
  } else if (tuple.size == 1) {
    result = tuple.elements[0];
    tuple_destroy(&tuple);
  } else {
    result = context_values_append(c, value_create_tuple(tuple));
  }

  return success(result);
}

static ParserResult unop(Parser *restrict p, Context *restrict c) {
  Token op = p->curtok;
  NEXTTOK(p);

  TRY(maybe, parse_precedence(p, c, PREC_UNARY));

  switch (op) {
  case TOK_MINUS: return success(context_emit_neg(c, maybe.result));

  default: EXP_UNREACHABLE();
  }
}

static ParserResult
binop(Parser *restrict p, Context *restrict c, Operand left) {
  Token op        = p->curtok;
  ParseRule *rule = get_rule(op);
  NEXTTOK(p); // eat the operator

  TRY(maybe, parse_precedence(p, c, (Precedence)(rule->precedence + 1)));
  Operand right = maybe.result;

  switch (op) {
  case TOK_DOT:     return success(context_emit_dot(c, left, right));
  case TOK_PLUS:    return success(context_emit_add(c, left, right));
  case TOK_MINUS:   return success(context_emit_sub(c, left, right));
  case TOK_STAR:    return success(context_emit_mul(c, left, right));
  case TOK_SLASH:   return success(context_emit_div(c, left, right));
  case TOK_PERCENT: return success(context_emit_mod(c, left, right));

  default: EXP_UNREACHABLE();
  }
}

static ParserResult parse_actual_argument_list(Parser *restrict p,
                                               Context *restrict c,
                                               Tuple *restrict argument_list) {
  // #note: the nil literal is spelled "()", which is
  // lexically identical to an empty argument list
  EXPECT(nil, TOK_NIL);
  if (nil.found) { return success(zero()); }

  NEXTTOK(p); // eat '('

  EXPECT(end_paren, TOK_END_PAREN);
  if (!end_paren.found) {
    bool comma_found = false;
    do {
      TRY(maybe, expression(p, c));

      tuple_append(argument_list, maybe.result);

      EXPECT(comma, TOK_COMMA);
      comma_found = comma.found;
    } while (comma_found);

    EXPECT(end_paren, TOK_END_PAREN);
    if (!end_paren.found) { return error(p, ERROR_PARSER_EXPECTED_END_PAREN); }
  }

  return success(zero());
}

static ParserResult
call(Parser *restrict p, Context *restrict c, Operand left) {
  Tuple argument_list = tuple_create();

  TRY(maybe, parse_actual_argument_list(p, c, &argument_list));

  Operand actual_arguments =
      context_values_append(c, value_create_tuple(argument_list));
  return success(context_emit_call(c, left, actual_arguments));
}

static ParserResult nil(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p);
  Operand idx = context_values_append(c, value_create_nil());
  return success(idx);
}

static ParserResult boolean_true(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p);
  Operand idx = context_values_append(c, value_create_boolean(1));
  return success(idx);
}

static ParserResult boolean_false(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p);
  Operand idx = context_values_append(c, value_create_boolean(0));
  return success(idx);
}

static ParserResult integer(Parser *restrict p,
                            [[maybe_unused]] Context *restrict c) {
  StringView sv = curtxt(p);
  i64 integer   = str_to_i64(sv.ptr, sv.length);

  NEXTTOK(p);
  Operand B;
  B = operand_immediate(integer);

  return success(B);
}

static ParserResult identifier(Parser *restrict p, Context *restrict c) {
  StringView name = context_intern(c, curtxt(p));
  NEXTTOK(p);

  LocalVariable *var = context_lookup_local(c, name);
  if (var != NULL) { return success(operand_ssa(var->ssa)); }

  SymbolTableElement *global = context_global_symbol_table_at(c, name);
  if (string_view_empty(global->name)) {
    return error(p, ERROR_TYPECHECK_UNDEFINED_SYMBOL);
  }

  u64 idx = context_global_labels_insert(c, name);
  return success(operand_label(idx));
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

  // Parse the right hand side of the expression
  TRY(right, rule->prefix(p, c));

  while (1) {
    ParseRule *rule = get_rule(p->curtok);

    if (precedence > rule->precedence) { break; }

    TRY(left, rule->infix(p, c, right.result));

    right = left;
  }

  return right;
}

static ParseRule *get_rule(Token token) {
  static ParseRule rules[] = {
      [TOK_END] = {         NULL,  NULL,   PREC_NONE},

      [TOK_ERROR_UNEXPECTED_CHAR]        = {         NULL,  NULL,   PREC_NONE},
      [TOK_ERROR_UNMATCHED_DOUBLE_QUOTE] = {         NULL,  NULL,   PREC_NONE},

      [TOK_BEGIN_COMMENT] = {         NULL,  NULL,   PREC_NONE},
      [TOK_END_COMMENT]   = {         NULL,  NULL,   PREC_NONE},
      [TOK_BEGIN_PAREN]   = {       parens,  call,   PREC_CALL},
      [TOK_END_PAREN]     = {         NULL,  NULL,   PREC_NONE},
      [TOK_BEGIN_BRACE]   = {         NULL,  NULL,   PREC_NONE},
      [TOK_COMMA]         = {         NULL,  NULL,   PREC_NONE},
      [TOK_DOT]           = {         NULL, binop,   PREC_CALL},
      [TOK_SEMICOLON]     = {         NULL,  NULL,   PREC_NONE},
      [TOK_COLON]         = {         NULL,  NULL,   PREC_NONE},
      [TOK_RIGHT_ARROW]   = {         NULL,  NULL,   PREC_NONE},

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

      [TOK_NIL]            = {          nil,  call,   PREC_CALL},
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
  ParserResult result = nexttok(&p);
  if (result.has_error) {
    error_print(&result.error, context_source_path(c), curline(&p));
    parser_result_destroy(&result);
    return EXIT_FAILURE;
  }

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

#undef TRY
#undef EXPECT
#undef NEXTTOK
