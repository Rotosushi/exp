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
#include "frontend/source_location.h"
#include "imr/operand.h"
#include "utility/io.h"
#include "utility/numeric_conversions.h"
#include "utility/unreachable.h"

typedef struct Parser {
  Lexer lexer;
  SourceLocation curloc;
  Token curtok;
} Parser;

static Parser parser_create() {
  Parser parser = {.lexer  = lexer_create(),
                   .curtok = TOK_END,
                   .curloc = source_location_create()};
  return parser;
}

static void parser_set_filename(Parser *restrict parser, StringView filename) {
  parser->curloc.file = filename;
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

static u64 curcol(Parser *restrict parser) {
  return lexer_current_column(&parser->lexer);
}

static bool peek(Parser *restrict parser, Token token) {
  return parser->curtok == token;
}

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

static ParserResult
failure(Parser *restrict p, ErrorCode code, String message) {
  ParserResult result = {.has_error = 1,
                         .error     = error(code, message, p->curloc)};
  return result;
}

static ParserResult syntax_error(Parser *restrict p, String message) {
  return failure(p, ERROR_SYNTAX, message);
}

static ParserResult semantic_error(Parser *restrict p, String message) {
  return failure(p, ERROR_SEMANTICS, message);
}

/*
static ParserResult precision_error(Parser *restrict p, String message) {
  return failure(p, ERROR_PRECISION, message);
}


static ParserResult precision_i64(Parser *restrict p) {
  String msg = string_create();
  string_append(&msg, SV("integral literal ["));
  string_append(&msg, curtxt(p));
  string_append(&msg, SV("] cannot be represented by integral type [i64]"));
  return precision_error(p, msg);
}
*/

static ParserResult expected_text(Parser *restrict p, StringView expect) {
  String msg = string_create();
  string_append(&msg, SV("expected ["));
  string_append(&msg, expect);
  string_append(&msg, SV("] actual ["));
  string_append(&msg, curtxt(p));
  string_append(&msg, SV("]"));
  return syntax_error(p, msg);
}

static ParserResult expected_token(Parser *restrict p, Token expect) {
  return expected_text(p, token_to_view(expect));
}

/*
static String mismatched_type(Type *expect, Type *actual) {
  String expect_str = string_create();
  String actual_str = string_create();
  print_type(expect, &expect_str);
  print_type(actual, &actual_str);

  String msg =
      expected_text(string_to_view(&expect_str), string_to_view(&actual_str));

  string_destroy(&expect_str);
  string_destroy(&actual_str);

  return msg;
}
*/

static ParserResult expected_type(Parser *restrict p) {
  String msg = string_create();
  string_append(&msg, SV("expected a Type expression, actual ["));
  string_append(&msg, curtxt(p));
  string_append(&msg, SV("]"));
  return syntax_error(p, msg);
}

static ParserResult expected_declaration(Parser *restrict p) {
  String msg = string_create();
  string_append(&msg, SV("expected a declaration, actual ["));
  string_append(&msg, curtxt(p));
  string_append(&msg, SV("]"));
  return syntax_error(p, msg);
}

static ParserResult expected_expression(Parser *restrict p) {
  String msg = string_create();
  string_append(&msg, SV("expected an expression, actual ["));
  string_append(&msg, curtxt(p));
  string_append(&msg, SV("]"));
  return syntax_error(p, msg);
}

static ParserResult undefined_identifier(Parser *restrict p, StringView id) {
  String msg = string_create();
  string_append(&msg, SV("identifier ["));
  string_append(&msg, id);
  string_append(&msg, SV("] is undefined in current scope"));
  return semantic_error(p, msg);
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
      return expected_token(parser, TOK_END_COMMENT);
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
  parser->curtok = lexer_scan(&parser->lexer);

  while (parser->curtok == TOK_BEGIN_COMMENT) {
    ParserResult result = comment(parser);
    if (result.has_error) { return result; }
  }

  parser->curloc.line   = curline(parser);
  parser->curloc.column = curcol(parser);
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

#define NEXTTOK(parser)                                                        \
  {                                                                            \
    ParserResult result = nexttok(parser);                                     \
    if (result.has_error) { return result; }                                   \
  }

#define TRY(name, call)                                                        \
  ParserResult name = call;                                                    \
  if (name.has_error) { return name; }

#define EXPECT_RESULT(name, token)                                             \
  ExpectResult name = expect(p, token);                                        \
  if (name.has_error) { return from_expect(name); }

#define EXPECT(token)                                                          \
  {                                                                            \
    EXPECT_RESULT(result, token);                                              \
    if (!result.found) { return expected_token(p, token); }                    \
  }

#define EXPECT_ID(name)                                                        \
  StringView name = string_view_create();                                      \
  {                                                                            \
    if (!peek(p, TOK_IDENTIFIER)) {                                            \
      return expected_token(p, TOK_IDENTIFIER);                                \
    }                                                                          \
    name = context_intern(c, curtxt(p));                                       \
    NEXTTOK(p);                                                                \
  }

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
  EXPECT_RESULT(nil, TOK_NIL);
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

    EXPECT_RESULT(comma, TOK_COMMA);
    found_comma = comma.found;
  } while (found_comma);

  EXPECT(TOK_END_PAREN);

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

  default: return expected_type(p);
  }

  NEXTTOK(p); // eat scalar-type
  return success(zero());
}

// formal-argument = identifier ":" type
static ParserResult parse_formal_argument(Parser *restrict p,
                                          Context *restrict c,
                                          FormalArgument *arg) {
  EXPECT_ID(name);

  EXPECT(TOK_COLON);

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
  EXPECT_RESULT(nil, TOK_NIL);
  if (nil.found) { return success(zero()); }

  EXPECT(TOK_BEGIN_PAREN);

  EXPECT_RESULT(end_paren, TOK_END_PAREN);
  if (!end_paren.found) {
    u8 index         = 0;
    bool comma_found = false;
    do {
      FormalArgument arg = {.index = index++};

      TRY(maybe, parse_formal_argument(p, c, &arg));

      function_body_new_argument(body, arg);

      EXPECT_RESULT(comma, TOK_COMMA);
      comma_found = comma.found;
    } while (comma_found);

    EXPECT(TOK_END_PAREN);
  }

  return success(zero());
}

// return = "return" expression ";"
static ParserResult return_(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat "return"

  TRY(maybe, expression(p, c));

  EXPECT(TOK_SEMICOLON);

  context_emit_return(c, maybe.result);
  return success(zero());
}

// constant = "const" identifier "=" expression ";"
static ParserResult constant(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat 'const'

  EXPECT_ID(name);

  EXPECT(TOK_EQUAL);

  TRY(exp, expression(p, c));

  EXPECT(TOK_SEMICOLON);

  context_def_constant_local(c, name, exp.result);
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

    EXPECT(TOK_SEMICOLON);

    return result;
  }
  }
}

static ParserResult parse_block(Parser *restrict p, Context *restrict c) {
  EXPECT(TOK_BEGIN_BRACE);

  EXPECT_RESULT(end_brace, TOK_END_BRACE);
  bool found_end_brace = end_brace.found;
  while (!found_end_brace) {
    TRY(maybe, statement(p, c));
    EXPECT_RESULT(end_brace, TOK_END_BRACE);
    found_end_brace = end_brace.found;
  }

  return success(zero());
}

static ParserResult function(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat "fn"

  EXPECT_ID(name);

  SymbolTableElement *ste = context_enter_global(c, name);
  assert(ste->kind == STE_UNDEFINED);
  ste->kind          = STE_FUNCTION;
  FunctionBody *body = &ste->function_body;

  { TRY(args, parse_formal_argument_list(p, c, body)); }

  EXPECT(TOK_RIGHT_ARROW);

  { TRY(block, parse_block(p, c)); }

#if EXP_DEBUG
  file_write(stderr, SV("\nparsed a function: \nfn "));
  file_write(stderr, name);
  write_function_body(stderr, body, c);
  file_write(stderr, SV("\n"));
#endif

  context_leave_global(c);
  return success(zero());
}

static ParserResult global_constant(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat 'const'

  EXPECT_ID(name);

  SymbolTableElement *ste = context_enter_global(c, name);
  assert(ste->kind == STE_UNDEFINED);
  ste->kind = STE_CONSTANT;

  EXPECT(TOK_EQUAL);

  TRY(exp, expression(p, c));

  EXPECT(TOK_SEMICOLON);

#if EXP_DEBUG
  file_write(stderr, SV("\nparsed a constant: \nconst "));
  file_write(stderr, name);
  file_write(stderr, SV("\n"));
#endif

  context_def_constant_global(c, name, exp.result);
  context_leave_global(c);
  return success(zero());
}

static ParserResult definition(Parser *restrict p, Context *restrict c) {
  switch (p->curtok) {
  case TOK_FN:    return function(p, c);
  case TOK_CONST: return global_constant(p, c);

  default: return expected_declaration(p);
  }
}

static ParserResult parens(Parser *restrict p, Context *restrict c) {
  NEXTTOK(p); // eat '('

  Tuple tuple      = tuple_create();
  bool found_comma = false;
  do {
    TRY(result, expression(p, c));
    tuple_append(&tuple, result.result);

    EXPECT_RESULT(comma, TOK_COMMA);
    found_comma = comma.found;
  } while (found_comma);

  EXPECT(TOK_END_PAREN);

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

  default: EXP_UNREACHABLE;
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

  default: EXP_UNREACHABLE;
  }
}

static ParserResult
parse_actual_argument_list(Parser *restrict p,
                           Context *restrict c,
                           ActualArgumentList *restrict list) {
  // #note: the nil literal is spelled "()", which is
  // lexically identical to an empty argument list
  EXPECT_RESULT(nil, TOK_NIL);
  if (nil.found) { return success(zero()); }

  NEXTTOK(p); // eat '('

  EXPECT_RESULT(end_paren, TOK_END_PAREN);
  if (!end_paren.found) {
    bool comma_found = false;
    do {
      TRY(maybe, expression(p, c));

      actual_argument_list_append(list, maybe.result);

      EXPECT_RESULT(comma, TOK_COMMA);
      comma_found = comma.found;
    } while (comma_found);

    EXPECT(TOK_END_PAREN);
  }

  return success(zero());
}

static ParserResult
call(Parser *restrict p, Context *restrict c, Operand left) {
  CallPair pair = context_new_call(c);

  TRY(maybe, parse_actual_argument_list(p, c, pair.list));

  return success(context_emit_call(c, left, operand_call(pair.index)));
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
  if (string_view_empty(global->name)) { return undefined_identifier(p, name); }

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
  if (rule->prefix == NULL) { return expected_expression(p); }

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
    write_error(&result.error, stderr);
    parser_result_destroy(&result);
    return EXIT_FAILURE;
  }

  while (!finished(&p)) {
    ParserResult maybe = definition(&p, c);
    if (maybe.has_error) {
      write_error(&result.error, stderr);
      parser_result_destroy(&maybe);
      return EXIT_FAILURE;
    }
  }

#if EXP_DEBUG
  file_write(stderr, SV("\nfn _init"));
  write_function_body(stderr, &context_global_init(c)->function_body, c);
  file_write(stderr, SV("\n"));
#endif

  return EXIT_SUCCESS;
}

i32 parse_source(Context *restrict c) {
  assert(c != NULL);
  StringView path = context_source_path(c);
  FILE *file      = file_open(path, SV("r"));
  String buffer   = string_from_file(file);
  file_close(file);
  i32 result = parse_buffer(string_to_cstring(&buffer), buffer.length, c);
  string_destroy(&buffer);
  return result;
}

#undef TRY
#undef EXPECT
#undef EXPECT_RESULT
#undef NEXTTOK
