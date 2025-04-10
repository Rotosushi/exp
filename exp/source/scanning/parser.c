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
#include "imr/operand.h"
#include "scanning/lexer.h"
#include "scanning/parser.h"
#include "scanning/token.h"
#include "support/io.h"
#include "support/message.h"
#include "support/numeric_conversions.h"
#include "support/unreachable.h"

typedef struct Parser {
    Lexer lexer;
    Token curtok;
} Parser;

/*
typedef struct ParserResult {
    bool has_error;
    union {
        Operand result;
        Error error;
    };
} ParserResult;

static void parser_result_destroy(ParserResult * pr) {
    if (pr->has_error) { error_destroy(&pr->error); }
}

static ParserResult error(Parser * p, ErrorCode code) {
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
*/

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

typedef bool (*PrefixFunction)(Operand *result,
                               Parser  *parser,
                               Context *context);
typedef bool (*InfixFunction)(Operand *result,
                              Operand  left,
                              Parser  *parser,
                              Context *context);

typedef struct ParseRule {
    PrefixFunction prefix;
    InfixFunction  infix;
    Precedence     precedence;
} ParseRule;

static Parser parser_create() {
    Parser parser;
    parser.lexer  = lexer_create();
    parser.curtok = TOK_END;
    return parser;
}

static void parser_set_view(Parser *parser, char const *buffer, u64 length) {
    assert(parser != NULL);
    assert(buffer != NULL);
    lexer_set_view(&(parser->lexer), buffer, length);
}

static bool finished(Parser *parser) { return parser->curtok == TOK_END; }

static StringView curtxt(Parser *parser) {
    return lexer_current_text(&parser->lexer);
}

static u64 curline(Parser *parser) {
    return lexer_current_line(&parser->lexer);
}

static bool error(Parser *p, Context *c, ErrorCode code) {
    Error *current_error = context_current_error(c);
    error_assign(current_error, code, lexer_current_text(&p->lexer));
    return false;
}

// static u64 curcol(Parser * parser) {
//   return lexer_current_column(&parser->lexer);
// }

static bool peek(Parser *parser, Token token) {
    return parser->curtok == token;
}

static bool comment(Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("comment:"), stderr);
        trace(curtxt(parser), stderr);
    }

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
            return error(parser, context, ERROR_PARSER_EXPECTED_END_COMMENT);
        }

        if (peek(parser, TOK_BEGIN_COMMENT)) {
            if (!comment(parser, context)) { return false; }
        }
    }

    // eat '*/'
    parser->curtok = lexer_scan(&parser->lexer);
    return true;
}

static bool nexttok(Parser *parser, Context *context) {
    if (lexer_at_end(&parser->lexer)) {
        parser->curtok = TOK_END;
        return true;
    }

    parser->curtok = lexer_scan(&parser->lexer);

    while (parser->curtok == TOK_BEGIN_COMMENT) {
        if (!comment(parser, context)) { return false; }
    }

    return true;
}

typedef enum ExpectResult {
    EXPECT_RESULT_SUCCESS,
    EXPECT_RESULT_TOKEN_NOT_FOUND,
    EXPECT_RESULT_FAILURE,
} ExpectResult;

static ExpectResult expect(Parser *parser, Context *context, Token token) {
    if (!peek(parser, token)) { return EXPECT_RESULT_TOKEN_NOT_FOUND; }
    if (!nexttok(parser, context)) { return EXPECT_RESULT_FAILURE; }
    return EXPECT_RESULT_SUCCESS;
}

static ParseRule *get_rule(Token token);
static bool       expression(Operand *result, Parser *parser, Context *context);
static bool       parse_precedence(Operand   *result,
                                   Precedence precedence,
                                   Parser    *parser,
                                   Context   *context);

static bool parse_type(Type const **result, Parser *parser, Context *context);

static bool
parse_tuple_type(Type const **result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("parse_tuple_type:"), stderr);
        trace(curtxt(parser), stderr);
    }

    // an empty tuple type is equivalent to a nil type.
    switch (expect(parser, context, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS: {
        *result = context_nil_type(context);
        return true;
    }

    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    assert(peek(parser, TOK_BEGIN_PAREN));
    if (!nexttok(parser, context)) { return false; } // eat '('

    TupleType tuple_type = tuple_type_create();

    bool found_comma = false;
    do {
        Type const *element = NULL;
        if (!parse_type(&element, parser, context)) { return false; }
        assert(element != NULL);

        tuple_type_append(&tuple_type, element);

        switch (expect(parser, context, TOK_COMMA)) {
        case EXPECT_RESULT_SUCCESS:         found_comma = true; break;
        case EXPECT_RESULT_TOKEN_NOT_FOUND: found_comma = false; break;
        case EXPECT_RESULT_FAILURE:         return false;
        default:                            EXP_UNREACHABLE();
        }
    } while (found_comma);

    switch (expect(parser, context, TOK_END_PAREN)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_END_PAREN);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    // a tuple type of length 1 is equivalent to that type.
    if (tuple_type.size == 1) {
        *result = tuple_type.types[0];
        tuple_type_destroy(&tuple_type);
    } else {
        *result = context_tuple_type(context, tuple_type);
    }

    return true;
}

static bool parse_type(Type const **result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("parse_type:"), stderr);
        trace(curtxt(parser), stderr);
    }

    switch (parser->curtok) {
    // composite types
    case TOK_BEGIN_PAREN: return parse_tuple_type(result, parser, context);

    // scalar types
    case TOK_NIL:       *result = context_nil_type(context); break;
    case TOK_TYPE_NIL:  *result = context_nil_type(context); break;
    case TOK_TYPE_BOOL: *result = context_boolean_type(context); break;
    case TOK_TYPE_I64:  *result = context_i64_type(context); break;

    default: return error(parser, context, ERROR_PARSER_EXPECTED_TYPE);
    }

    if (!nexttok(parser, context)) { return false; } // eat scalar-type
    return true;
}

// formal-argument = identifier ":" type
static bool
parse_formal_argument(FormalArgument *arg, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("parse_formal_argument:"), stderr);
        trace(curtxt(parser), stderr);
    }

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, context, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    StringView name = context_intern(context, curtxt(parser));
    if (!nexttok(parser, context)) { return false; }

    switch (expect(parser, context, TOK_COLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_COLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    Type const *type = NULL;
    if (!parse_type(&type, parser, context)) { return false; }
    assert(type != NULL);

    arg->name = name;
    arg->type = type;
    return true;
}

// formal-argument-list = "(" (formal-argument ("," formal-argument)*)? ")"
static bool
parse_formal_argument_list(Function *body, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("parse_formal_argument_list:"), stderr);
        trace(curtxt(parser), stderr);
    }
    // #note: the nil literal is spelled "()", which is
    // lexically identical to an empty argument list. so we parse it as such
    switch (expect(parser, context, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    switch (expect(parser, context, TOK_BEGIN_PAREN)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_BEGIN_PAREN);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    switch (expect(parser, context, TOK_END_PAREN)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        u8   index       = 0;
        bool comma_found = false;
        do {
            FormalArgument arg = {.index = index++};

            if (!parse_formal_argument(&arg, parser, context)) { return false; }

            function_new_argument(body, arg);

            switch (expect(parser, context, TOK_COMMA)) {
            case EXPECT_RESULT_SUCCESS:         comma_found = true; break;
            case EXPECT_RESULT_TOKEN_NOT_FOUND: comma_found = false; break;
            case EXPECT_RESULT_FAILURE:         return false;
            default:                            EXP_UNREACHABLE();
            }
        } while (comma_found);

        switch (expect(parser, context, TOK_END_PAREN)) {
        case EXPECT_RESULT_SUCCESS: break;
        case EXPECT_RESULT_TOKEN_NOT_FOUND:
            return error(parser, context, ERROR_PARSER_EXPECTED_END_PAREN);
        case EXPECT_RESULT_FAILURE: return false;
        default:                    EXP_UNREACHABLE();
        }
        return true;
    }

    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }
}

// return = "return" expression ";"
static bool return_(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("return_:"), stderr);
        trace(curtxt(parser), stderr);
    }

    if (!nexttok(parser, context)) { return false; } // eat "return"

    if (!expression(result, parser, context)) { return false; }

    switch (expect(parser, context, TOK_SEMICOLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_SEMICOLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    context_emit_return(context, *result);
    return true;
}

// constant = "const" identifier "=" expression ";"
static bool constant(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("constant:"), stderr);
        trace(curtxt(parser), stderr);
    }

    if (!nexttok(parser, context)) { return false; } // eat 'const'

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, context, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }
    StringView name = context_intern(context, curtxt(parser));
    if (!nexttok(parser, context)) { return false; }

    switch (expect(parser, context, TOK_EQUAL)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_EQUAL);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    if (!expression(result, parser, context)) { return false; }

    switch (expect(parser, context, TOK_SEMICOLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_SEMICOLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    if (context_trace(context) && context_prolix(context)) {
        String buffer = string_create();
        string_append(&buffer, SV("parsed a constant definition: "));
        string_append(&buffer, name);
        string_append(&buffer, SV(" = "));
        print_operand(&buffer, *result, context);
        string_append(&buffer, SV("\n"));
        trace(string_to_view(&buffer), stdout);
        string_destroy(&buffer);
    }

    context_def_local_const(context, name, *result);
    return true;
}

// statement = return
//           | constant
//           | expression
static bool statement(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("statement:"), stderr);
        trace(curtxt(parser), stderr);
    }

    switch (parser->curtok) {
    case TOK_RETURN: return return_(result, parser, context);
    case TOK_CONST:  return constant(result, parser, context);

    default: {
        if (!expression(result, parser, context)) { return false; }

        switch (expect(parser, context, TOK_SEMICOLON)) {
        case EXPECT_RESULT_SUCCESS: break;
        case EXPECT_RESULT_TOKEN_NOT_FOUND:
            return error(parser, context, ERROR_PARSER_EXPECTED_SEMICOLON);
        case EXPECT_RESULT_FAILURE: return false;
        default:                    EXP_UNREACHABLE();
        }

        return true;
    }
    }
}

static bool parse_block(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("parse_block:"), stderr);
        trace(curtxt(parser), stderr);
    }

    switch (expect(parser, context, TOK_BEGIN_BRACE)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_BEGIN_BRACE);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    switch (expect(parser, context, TOK_END_BRACE)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        bool found_end_brace = false;
        while (!found_end_brace) {
            if (!statement(result, parser, context)) { return false; }
            switch (expect(parser, context, TOK_END_BRACE)) {
            case EXPECT_RESULT_SUCCESS:         found_end_brace = true; break;
            case EXPECT_RESULT_TOKEN_NOT_FOUND: found_end_brace = false; break;
            case EXPECT_RESULT_FAILURE:         return false;
            default:                            EXP_UNREACHABLE();
            }
        }
        return true;
    }

    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }
}

static bool function(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("function:"), stderr);
        trace(curtxt(parser), stderr);
    }

    if (!nexttok(parser, context)) { return false; } // eat "fn"

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, context, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    StringView name = context_intern(context, curtxt(parser));
    if (!nexttok(parser, context)) { return false; }

    Function *body = context_enter_function(context, name);

    if (!parse_formal_argument_list(body, parser, context)) { return false; }

    switch (expect(parser, context, TOK_RIGHT_ARROW)) {
    case EXPECT_RESULT_SUCCESS:
        if (!parse_type(&body->return_type, parser, context)) { return false; }
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    if (!parse_block(result, parser, context)) { return false; }

    if (context_trace(context) && context_prolix(context)) {
        String buffer = string_create();
        string_append(&buffer, SV("parsed a function definition: "));
        string_append(&buffer, name);
        print_function(&buffer, body, context);
        string_append(&buffer, SV("\n"));
        trace(string_to_view(&buffer), stdout);
        string_destroy(&buffer);
    }

    context_leave_function(context);
    return true;
}

static bool definition(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("definition:"), stderr);
        trace(curtxt(parser), stderr);
    }

    switch (parser->curtok) {
    case TOK_FN: return function(result, parser, context);

    default: return error(parser, context, ERROR_PARSER_EXPECTED_KEYWORD_FN);
    }
}

static bool parse_tuple(Tuple *tuple, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("parse_tuple:"), stderr);
        trace(curtxt(parser), stderr);
    }

    switch (expect(parser, context, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    assert(peek(parser, TOK_BEGIN_PAREN));
    if (!nexttok(parser, context)) { return false; }

    switch (expect(parser, context, TOK_END_PAREN)) {
    case EXPECT_RESULT_SUCCESS:         break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        bool found_comma = false;
        do {
            Operand result;
            if (!expression(&result, parser, context)) { return false; }
            tuple_append(tuple, result);

            switch (expect(parser, context, TOK_COMMA)) {
            case EXPECT_RESULT_SUCCESS:         found_comma = true; break;
            case EXPECT_RESULT_TOKEN_NOT_FOUND: found_comma = false; break;
            case EXPECT_RESULT_FAILURE:         {
                tuple_destroy(tuple);
                return false;
            }
            default: EXP_UNREACHABLE();
            }
        } while (found_comma);

        switch (expect(parser, context, TOK_END_PAREN)) {
        case EXPECT_RESULT_SUCCESS: break;
        case EXPECT_RESULT_TOKEN_NOT_FOUND:
            return error(parser, context, ERROR_PARSER_EXPECTED_END_PAREN);
        case EXPECT_RESULT_FAILURE: return false;
        default:                    EXP_UNREACHABLE();
        }
        break;
    }
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    return true;
}

static bool parens(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("parens:"), stderr);
        trace(curtxt(parser), stderr);
    }

    Tuple tuple = tuple_create();

    if (!parse_tuple(&tuple, parser, context)) { return false; };

    if (tuple.size == 0) {
        *result = context_constants_append(context, value_create_nil());
        tuple_destroy(&tuple);
    } else if (tuple.size == 1) {
        *result = tuple.elements[0];
        tuple_destroy(&tuple);
    } else {
        *result = context_constants_append(context, value_create_tuple(tuple));
    }

    return true;
}

static bool unop(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("unop:"), stderr);
        trace(curtxt(parser), stderr);
    }
    Token op = parser->curtok;
    if (!nexttok(parser, context)) { return false; }

    if (!parse_precedence(result, PREC_UNARY, parser, context)) {
        return false;
    }

    switch (op) {
    case TOK_MINUS: *result = context_emit_negate(context, *result); break;
    default:        EXP_UNREACHABLE();
    }
    return true;
}

static bool
binop(Operand *result, Operand left, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("binop:"), stderr);
        trace(curtxt(parser), stderr);
    }
    Token      op   = parser->curtok;
    ParseRule *rule = get_rule(op);
    if (!nexttok(parser, context)) { return false; } // eat the operator

    if (!parse_precedence(
            result, (Precedence)(rule->precedence + 1), parser, context)) {
        return false;
    }
    Operand right = *result;

    switch (op) {
    case TOK_DOT:  *result = context_emit_dot(context, left, right); break;
    case TOK_PLUS: *result = context_emit_add(context, left, right); break;
    case TOK_MINUS:
        *result = context_emit_subtract(context, left, right);
        break;
    case TOK_STAR:  *result = context_emit_multiply(context, left, right); break;
    case TOK_SLASH: *result = context_emit_divide(context, left, right); break;
    case TOK_PERCENT:
        *result = context_emit_modulus(context, left, right);
        break;

    default: EXP_UNREACHABLE();
    }
    return true;
}

static bool
call(Operand *result, Operand left, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("call:"), stderr);
        trace(curtxt(parser), stderr);
    }
    Tuple argument_list = tuple_create();

    if (!parse_tuple(&argument_list, parser, context)) { return false; }

    Operand actual_arguments =
        context_constants_append(context, value_create_tuple(argument_list));

    *result = context_emit_call(context, left, actual_arguments);
    return true;
}

static bool nil(Operand *result, Parser *parser, Context *context) {
    assert(peek(parser, TOK_NIL));
    if (context_trace(context)) {
        trace(SV("nil:"), stderr);
        trace(curtxt(parser), stderr);
    }
    if (!nexttok(parser, context)) { return false; }
    *result = context_constants_append(context, value_create_nil());
    return true;
}

static bool boolean_true(Operand *result, Parser *parser, Context *context) {
    assert(peek(parser, TOK_TRUE));
    if (context_trace(context)) {
        trace(SV("boolean_true:"), stderr);
        trace(curtxt(parser), stderr);
    }
    if (!nexttok(parser, context)) { return false; }
    *result = context_constants_append(context, value_create_boolean(true));
    return true;
}

static bool boolean_false(Operand *result, Parser *parser, Context *context) {
    assert(peek(parser, TOK_FALSE));
    if (context_trace(context)) {
        trace(SV("boolean_false:"), stderr);
        trace(curtxt(parser), stderr);
    }
    if (!nexttok(parser, context)) { return false; }
    *result = context_constants_append(context, value_create_boolean(0));
    return true;
}

static bool integer(Operand *result, Parser *parser, Context *context) {
    assert(peek(parser, TOK_INTEGER));
    if (context_trace(context)) {
        trace(SV("integer:"), stderr);
        trace(curtxt(parser), stderr);
    }

    StringView sv      = curtxt(parser);
    u64        integer = 0;

    if (!str_to_u64(&integer, sv.ptr, sv.length)) {
        return error(
            parser, context, ERROR_PARSER_INTEGER_LITERAL_OUT_OF_RANGE);
    }

    if (!nexttok(parser, context)) { return false; }
    if (u64_in_range_i64(integer)) {
        *result = operand_i64((i64)integer);
    } else {
        *result = operand_u64(integer);
    }

    return true;
}

static bool identifier(Operand *result, Parser *parser, Context *context) {
    assert(peek(parser, TOK_IDENTIFIER));
    if (context_trace(context)) {
        trace(SV("identifier:"), stderr);
        trace(curtxt(parser), stderr);
    }

    StringView name = context_intern(context, curtxt(parser));
    if (!nexttok(parser, context)) { return false; }

    LocalVariable *var = context_lookup_local(context, name);
    if (var != NULL) {
        *result = operand_ssa(var->ssa);
        return true;
    }

    Symbol *global = context_global_symbol_table_at(context, name);
    if (string_view_empty(global->name)) {
        return error(parser, context, ERROR_TYPECHECK_UNDEFINED_SYMBOL);
    }

    u32 index = context_labels_insert(context, name);
    *result   = operand_label(index);
    return true;
}

static bool expression(Operand *result, Parser *parser, Context *context) {
    if (context_trace(context)) {
        trace(SV("expression:"), stderr);
        trace(curtxt(parser), stderr);
    }
    return parse_precedence(result, PREC_ASSIGNMENT, parser, context);
}

static bool parse_precedence(Operand   *result,
                             Precedence precedence,
                             Parser    *parser,
                             Context   *context) {
    ParseRule *rule = get_rule(parser->curtok);
    if (rule->prefix == NULL) {
        return error(parser, context, ERROR_PARSER_EXPECTED_EXPRESSION);
    }
    if (context_trace(context)) {
        trace(SV("parse_precedence:"), stderr);
        trace(SV("precedence: "), stderr);
        trace_u64(precedence, stderr);
        trace(SV("prefix: "), stderr);
        trace(curtxt(parser), stderr);
    }

    // Parse the left hand side of the expression
    if (!rule->prefix(result, parser, context)) { return false; }

    while (1) {
        if (context_trace(context)) {
            trace(SV("parse_precedence: "), stderr);
            trace(SV("precedence: "), stderr);
            trace_u64(precedence, stderr);
            trace(SV("infix: "), stderr);
            trace(curtxt(parser), stderr);
        }

        ParseRule *rule = get_rule(parser->curtok);

        if (precedence > rule->precedence) { break; }

        Operand left;
        if (!rule->infix(&left, *result, parser, context)) { return false; }

        *result = left;
    }

    return true;
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

i32 parse_buffer(char const *buffer, u64 length, Context *c) {
    assert(buffer != NULL);
    assert(c != NULL);

    Parser p = parser_create();

    parser_set_view(&p, buffer, length);
    if (!nexttok(&p, c)) {
        error_print(
            context_current_error(c), context_source_path(c), curline(&p));
        return EXIT_FAILURE;
    }

    while (!finished(&p)) {
        Operand result;
        if (!definition(&result, &p, c)) {
            error_print(
                context_current_error(c), context_source_path(c), curline(&p));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

i32 parse_source(Context *c) {
    assert(c != NULL);
    if (context_prolix(c) || context_trace(c)) {
        trace(SV("parsing source: "), stderr);
        trace(context_source_path(c), stderr);
    }
    StringView path   = context_source_path(c);
    FILE      *file   = file_open(path.ptr, "r");
    String     buffer = string_from_file(file);
    file_close(file);
    i32 result = parse_buffer(string_to_cstring(&buffer), buffer.length, c);
    string_destroy(&buffer);
    return result;
}

#undef TRY
#undef EXPECT
#undef NEXTTOK
