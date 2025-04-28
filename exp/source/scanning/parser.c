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

typedef bool (*PrefixFunction)(Operand *restrict result,
                               Parser *restrict parser,
                               Context *restrict context);
typedef bool (*InfixFunction)(Operand *restrict result,
                              Operand left,
                              Parser *restrict parser,
                              Context *restrict context);

typedef struct ParseRule {
    PrefixFunction prefix;
    InfixFunction  infix;
    Precedence     precedence;
} ParseRule;

static void parser_create(Parser *restrict parser) {
    assert(parser != NULL);
    lexer_init(&(parser->lexer));
    parser->curtok = TOK_END;
}

static void parser_set_view(Parser *restrict parser,
                            char const *restrict buffer,
                            u64 length) {
    assert(parser != NULL);
    assert(buffer != NULL);
    lexer_set_view(&(parser->lexer), buffer, length);
}

static bool finished(Parser const *restrict parser) {
    return parser->curtok == TOK_END;
}

static StringView curtxt(Parser const *restrict parser) {
    return lexer_current_text(&parser->lexer);
}

static u64 curline(Parser const *restrict parser) {
    return lexer_current_line(&parser->lexer);
}

static bool error(Parser const *restrict parser,
                  Context *restrict context,
                  ErrorCode code) {
    Error *current_error = context_current_error(context);
    error_assign(current_error, code, lexer_current_text(&parser->lexer));
    return false;
}

static bool peek(Parser const *restrict parser, Token token) {
    return parser->curtok == token;
}

static bool nexttok(Parser *restrict parser) {
    if (lexer_at_end(&parser->lexer)) {
        parser->curtok = TOK_END;
        return true;
    }

    parser->curtok = lexer_scan(&parser->lexer);

    return true;
}

typedef enum ExpectResult {
    EXPECT_RESULT_SUCCESS,
    EXPECT_RESULT_TOKEN_NOT_FOUND,
    EXPECT_RESULT_FAILURE,
} ExpectResult;

static ExpectResult expect(Parser *restrict parser, Token token) {
    if (!peek(parser, token)) { return EXPECT_RESULT_TOKEN_NOT_FOUND; }
    if (!nexttok(parser)) { return EXPECT_RESULT_FAILURE; }
    return EXPECT_RESULT_SUCCESS;
}

static ParseRule *get_rule(Token token);
static bool       expression(Operand *restrict result,
                             Parser *restrict parser,
                             Context *restrict context);
static bool       parse_precedence(Operand *restrict result,
                                   Precedence precedence,
                                   Parser *restrict parser,
                                   Context *restrict context);

static bool parse_type(Type const **restrict result,
                       Parser *restrict parser,
                       Context *restrict context);

static bool parse_tuple_type(Type const **restrict result,
                             Parser *restrict parser,
                             Context *restrict context) {

    // an empty tuple type is equivalent to a nil type.
    switch (expect(parser, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS: {
        *result = context_nil_type(context);
        return true;
    }

    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    assert(peek(parser, TOK_BEGIN_PAREN));
    if (!nexttok(parser)) { return false; } // eat '('

    TupleType tuple_type = tuple_type_create();

    bool found_comma = false;
    do {
        Type const *element = NULL;
        if (!parse_type(&element, parser, context)) { return false; }
        assert(element != NULL);

        tuple_type_append(&tuple_type, element);

        switch (expect(parser, TOK_COMMA)) {
        case EXPECT_RESULT_SUCCESS:         found_comma = true; break;
        case EXPECT_RESULT_TOKEN_NOT_FOUND: found_comma = false; break;
        case EXPECT_RESULT_FAILURE:         return false;
        default:                            EXP_UNREACHABLE();
        }
    } while (found_comma);

    switch (expect(parser, TOK_END_PAREN)) {
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

static bool parse_type(Type const **restrict result,
                       Parser *restrict parser,
                       Context *restrict context) {

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

    if (!nexttok(parser)) { return false; } // eat scalar-type
    return true;
}

// formal-argument = identifier ":" type
static bool parse_formal_argument(Local *restrict arg,
                                  Parser *restrict parser,
                                  Context *restrict context) {

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, context, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    ConstantString *name = context_intern(context, curtxt(parser));
    if (!nexttok(parser)) { return false; }

    switch (expect(parser, TOK_COLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_COLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    Type const *type = NULL;
    if (!parse_type(&type, parser, context)) { return false; }
    assert(type != NULL);

    arg->name = constant_string_to_view(name);
    arg->type = type;
    return true;
}

// formal-argument-list = "(" (formal-argument ("," formal-argument)*)? ")"
static bool parse_formal_argument_list(Function *restrict body,
                                       Parser *restrict parser,
                                       Context *restrict context) {
    // #note: the nil literal is spelled "()", which is
    // lexically identical to an empty argument list. so we parse it as such
    switch (expect(parser, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    switch (expect(parser, TOK_BEGIN_PAREN)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_BEGIN_PAREN);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    switch (expect(parser, TOK_END_PAREN)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        bool comma_found = false;
        do {
            Local *arg = function_declare_argument(body);

            if (!parse_formal_argument(arg, parser, context)) { return false; }

            switch (expect(parser, TOK_COMMA)) {
            case EXPECT_RESULT_SUCCESS:         comma_found = true; break;
            case EXPECT_RESULT_TOKEN_NOT_FOUND: comma_found = false; break;
            case EXPECT_RESULT_FAILURE:         return false;
            default:                            EXP_UNREACHABLE();
            }
        } while (comma_found);

        switch (expect(parser, TOK_END_PAREN)) {
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
static bool return_(Operand *restrict result,
                    Parser *restrict parser,
                    Context *restrict context) {

    if (!nexttok(parser)) { return false; } // eat "return"

    if (!expression(result, parser, context)) { return false; }

    switch (expect(parser, TOK_SEMICOLON)) {
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
static bool let(Operand *restrict result,
                Parser *restrict parser,
                Context *restrict context) {
    if (!nexttok(parser)) { return false; } // eat 'let'

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, context, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }
    ConstantString *name = context_intern(context, curtxt(parser));
    if (!nexttok(parser)) { return false; }

    switch (expect(parser, TOK_EQUAL)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_EQUAL);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    if (!expression(result, parser, context)) { return false; }

    switch (expect(parser, TOK_SEMICOLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_SEMICOLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    Operand A = context_emit_load(context, *result);
    assert(A.kind == OPERAND_KIND_SSA);
    Local *local = context_lookup_local(context, A.data.ssa);
    local->name  = constant_string_to_view(name);

    return true;
}

// statement = return
//           | let
//           | expression
static bool statement(Operand *restrict result,
                      Parser *restrict parser,
                      Context *restrict context) {

    switch (parser->curtok) {
    case TOK_RETURN: return return_(result, parser, context);
    case TOK_LET:    return let(result, parser, context);

    default: {
        if (!expression(result, parser, context)) { return false; }

        switch (expect(parser, TOK_SEMICOLON)) {
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

static bool parse_block(Operand *restrict result,
                        Parser *restrict parser,
                        Context *restrict context) {

    switch (expect(parser, TOK_BEGIN_BRACE)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, context, ERROR_PARSER_EXPECTED_BEGIN_BRACE);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    switch (expect(parser, TOK_END_BRACE)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        bool found_end_brace = false;
        while (!found_end_brace) {
            if (!statement(result, parser, context)) { return false; }
            switch (expect(parser, TOK_END_BRACE)) {
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

static bool function(Operand *restrict result,
                     Parser *restrict parser,
                     Context *restrict context) {

    if (!nexttok(parser)) { return false; } // eat "fn"

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, context, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    ConstantString *name = context_intern(context, curtxt(parser));
    if (!nexttok(parser)) { return false; }

    Function *body =
        context_enter_function(context, constant_string_to_view(name));

    if (!parse_formal_argument_list(body, parser, context)) { return false; }

    switch (expect(parser, TOK_RIGHT_ARROW)) {
    case EXPECT_RESULT_SUCCESS:
        if (!parse_type(&body->return_type, parser, context)) { return false; }
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    if (!parse_block(result, parser, context)) { return false; }

    context_leave_function(context);
    return true;
}

static bool definition(Operand *restrict result,
                       Parser *restrict parser,
                       Context *restrict context) {

    switch (parser->curtok) {
    case TOK_FN: return function(result, parser, context);

    default: return error(parser, context, ERROR_PARSER_EXPECTED_KEYWORD_FN);
    }
}

static bool parse_tuple(Tuple *restrict tuple,
                        Parser *restrict parser,
                        Context *restrict context) {

    switch (expect(parser, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    assert(peek(parser, TOK_BEGIN_PAREN));
    if (!nexttok(parser)) { return false; }

    switch (expect(parser, TOK_END_PAREN)) {
    case EXPECT_RESULT_SUCCESS:         break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        bool found_comma = false;
        do {
            Operand result;
            if (!expression(&result, parser, context)) { return false; }
            tuple_append(tuple, result);

            switch (expect(parser, TOK_COMMA)) {
            case EXPECT_RESULT_SUCCESS:         found_comma = true; break;
            case EXPECT_RESULT_TOKEN_NOT_FOUND: found_comma = false; break;
            case EXPECT_RESULT_FAILURE:         {
                tuple_destroy(tuple);
                return false;
            }
            default: EXP_UNREACHABLE();
            }
        } while (found_comma);

        switch (expect(parser, TOK_END_PAREN)) {
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

static bool parens(Operand *restrict result,
                   Parser *restrict parser,
                   Context *restrict context) {

    Tuple tuple;
    tuple_create(&tuple);

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

static bool unop(Operand *restrict result,
                 Parser *restrict parser,
                 Context *restrict context) {
    Token op = parser->curtok;
    if (!nexttok(parser)) { return false; }

    if (!parse_precedence(result, PREC_UNARY, parser, context)) {
        return false;
    }

    switch (op) {
    case TOK_MINUS: *result = context_emit_negate(context, *result); break;
    default:        EXP_UNREACHABLE();
    }
    return true;
}

static bool binop(Operand *restrict result,
                  Operand left,
                  Parser *restrict parser,
                  Context *restrict context) {
    Token      op   = parser->curtok;
    ParseRule *rule = get_rule(op);
    if (!nexttok(parser)) { return false; } // eat the operator

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

static bool call(Operand *restrict result,
                 Operand left,
                 Parser *restrict parser,
                 Context *restrict context) {
    Tuple argument_list;
    tuple_create(&argument_list);

    if (!parse_tuple(&argument_list, parser, context)) { return false; }

    Operand actual_arguments =
        context_constants_append(context, value_create_tuple(argument_list));

    *result = context_emit_call(context, left, actual_arguments);
    return true;
}

static bool nil(Operand *restrict result,
                Parser *restrict parser,
                Context *restrict context) {
    assert(peek(parser, TOK_NIL));
    if (!nexttok(parser)) { return false; }
    *result = context_constants_append(context, value_create_nil());
    return true;
}

static bool boolean_true(Operand *restrict result,
                         Parser *restrict parser,
                         Context *restrict context) {
    assert(peek(parser, TOK_TRUE));
    if (!nexttok(parser)) { return false; }
    *result = context_constants_append(context, value_create_boolean(true));
    return true;
}

static bool boolean_false(Operand *restrict result,
                          Parser *restrict parser,
                          Context *restrict context) {
    assert(peek(parser, TOK_FALSE));
    if (!nexttok(parser)) { return false; }
    *result = context_constants_append(context, value_create_boolean(0));
    return true;
}

static bool integer(Operand *restrict result,
                    Parser *restrict parser,
                    Context *restrict context) {
    assert(peek(parser, TOK_INTEGER));
    StringView sv      = curtxt(parser);
    u64        integer = 0;

    if (!str_to_u64(&integer, sv.ptr, sv.length)) {
        return error(
            parser, context, ERROR_PARSER_INTEGER_LITERAL_OUT_OF_RANGE);
    }

    if (!nexttok(parser)) { return false; }
    if (u64_in_range_i64(integer)) {
        *result = operand_i64((i64)integer);
    } else {
        *result = operand_u64(integer);
    }

    return true;
}

static bool identifier(Operand *restrict result,
                       Parser *restrict parser,
                       Context *restrict context) {
    assert(peek(parser, TOK_IDENTIFIER));

    ConstantString *name = context_intern(context, curtxt(parser));
    if (!nexttok(parser)) { return false; }

    Local *local =
        context_lookup_local_name(context, constant_string_to_view(name));
    if (local != NULL) {
        *result = operand_ssa(local->ssa);
        return true;
    }

    Symbol *global =
        context_global_symbol_table_at(context, constant_string_to_view(name));
    if (string_view_empty(global->name)) {
        return error(parser, context, ERROR_ANALYSIS_UNDEFINED_SYMBOL);
    }

    *result = operand_label(name);
    return true;
}

static bool expression(Operand *restrict result,
                       Parser *restrict parser,
                       Context *restrict context) {
    return parse_precedence(result, PREC_ASSIGNMENT, parser, context);
}

static bool parse_precedence(Operand *restrict result,
                             Precedence precedence,
                             Parser *restrict parser,
                             Context *restrict context) {
    ParseRule *rule = get_rule(parser->curtok);
    if (rule->prefix == NULL) {
        return error(parser, context, ERROR_PARSER_EXPECTED_EXPRESSION);
    }
    // Parse the left hand side of the expression
    if (!rule->prefix(result, parser, context)) { return false; }

    while (1) {

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

        [TOK_BEGIN_PAREN] = {       parens,  call,   PREC_CALL},
        [TOK_END_PAREN]   = {         NULL,  NULL,   PREC_NONE},
        [TOK_BEGIN_BRACE] = {         NULL,  NULL,   PREC_NONE},
        [TOK_COMMA]       = {         NULL,  NULL,   PREC_NONE},
        [TOK_DOT]         = {         NULL, binop,   PREC_CALL},
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
        [TOK_LET]    = {         NULL,  NULL,   PREC_NONE},
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

i32 parse_buffer(char const *restrict buffer,
                 u64 length,
                 Context *restrict context) {
    assert(buffer != NULL);
    assert(context != NULL);

    Parser parser;
    parser_create(&parser);

    parser_set_view(&parser, buffer, length);
    if (!nexttok(&parser)) {
        error_print(context_current_error(context),
                    context_source_path(context),
                    curline(&parser));
        return EXIT_FAILURE;
    }

    while (!finished(&parser)) {
        Operand result;
        if (!definition(&result, &parser, context)) {
            error_print(context_current_error(context),
                        context_source_path(context),
                        curline(&parser));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

i32 parse_source(Context *restrict context) {
    assert(context != NULL);
    StringView path   = context_source_path(context);
    FILE      *file   = file_open(path.ptr, "r");
    String     buffer = string_from_file(file);
    file_close(file);
    i32 result =
        parse_buffer(string_to_cstring(&buffer), buffer.length, context);
    string_destroy(&buffer);
    return result;
}

#undef TRY
#undef EXPECT
#undef NEXTTOK
