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
#include "support/assert.h"
#include "support/constant_string.h"
#include "support/numeric_conversions.h"
#include "support/unreachable.h"

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
    PREC_CALL,       // . () "\"
    PREC_PRIMARY,
} Precedence;

typedef bool (*PrefixFunction)(Operand *restrict result,
                               Parser *restrict parser);
typedef bool (*InfixFunction)(Operand *restrict result,
                              Operand left,
                              Parser *restrict parser);

typedef struct ParseRule {
    PrefixFunction prefix;
    InfixFunction  infix;
    Precedence     precedence;
} ParseRule;

void parser_create(Parser *restrict parser, Context *restrict context) {
    assert(parser != NULL);
    parser->context  = context;
    parser->function = NULL;
    lexer_init(&(parser->lexer));
    parser->curtok = TOK_END;
}

void parser_set_file(Parser *restrict parser, StringView file) {
    assert(parser != NULL);
    lexer_set_file(&parser->lexer, file);
}

void parser_current_source_location(Parser const *restrict parser,
                                    SourceLocation *restrict source_location) {
    assert(parser != NULL);
    assert(source_location != NULL);
    lexer_current_source_location(&parser->lexer, source_location);
}

bool parser_done(Parser const *restrict parser) {
    return parser->curtok == TOK_END;
}

static StringView curtxt(Parser const *restrict parser) {
    return lexer_current_text(&parser->lexer);
}

static u64 curline(Parser const *restrict parser) {
    return lexer_current_line(&parser->lexer);
}

static bool error(Parser const *restrict parser, ErrorCode code) {
    context_failure(parser->context, code, lexer_current_text(&parser->lexer));
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

bool parser_setup(Parser *restrict parser, StringView view) {
    assert(parser != NULL);
    assert(!string_view_empty(view));
    lexer_set_view(&(parser->lexer), view);
    if (!nexttok(parser)) {
        context_print_error(parser->context,
                            context_source_path(parser->context),
                            curline(parser));
        return false;
    }

    return true;
}

static void parser_emit_instruction(Parser *restrict parser,
                                    Instruction instruction) {
    assert(parser != NULL);
    assert(parser->function != NULL);
    bytecode_append(&parser->function->body, instruction);
}

static Operand parser_declare_local(Parser *restrict parser) {
    assert(parser != NULL);
    assert(parser->function != NULL);
    return operand_ssa(function_declare_local(parser->function));
}

static void parser_emit_return(Parser *restrict parser, Operand result) {
    parser_emit_instruction(parser, instruction_return(result));
}

static Operand
parser_emit_call(Parser *restrict parser, Operand callee, Operand arguments) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser,
                            instruction_call(result, callee, arguments));
    return result;
}

// static Operand parser_emit_fn(Parser *restrict parser, Operand fn) {
//     Operand result = parser_declare_local(parser);
//     parser_emit_instruction(parser, instruction_fn(result, fn));
//     return result;
// }

static Operand parser_emit_let(Parser *restrict parser, Operand value) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_let(result, value));
    return result;
}

static Operand parser_emit_neg(Parser *restrict parser, Operand value) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_neg(result, value));
    return result;
}

static Operand parser_emit_dot(Parser *restrict parser, Operand B, Operand C) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_dot(result, B, C));
    return result;
}

static Operand parser_emit_add(Parser *restrict parser, Operand B, Operand C) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_add(result, B, C));
    return result;
}

static Operand parser_emit_sub(Parser *restrict parser, Operand B, Operand C) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_sub(result, B, C));
    return result;
}

static Operand parser_emit_mul(Parser *restrict parser, Operand B, Operand C) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_mul(result, B, C));
    return result;
}

static Operand parser_emit_div(Parser *restrict parser, Operand B, Operand C) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_div(result, B, C));
    return result;
}

static Operand parser_emit_mod(Parser *restrict parser, Operand B, Operand C) {
    Operand result = parser_declare_local(parser);
    parser_emit_instruction(parser, instruction_mod(result, B, C));
    return result;
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
static bool       expression(Operand *restrict result, Parser *restrict parser);
static bool       parse_precedence(Operand *restrict result,
                                   Precedence precedence,
                                   Parser *restrict parser);

static bool parse_type(Type const **restrict result, Parser *restrict parser);

static bool parse_tuple_type(Type const **restrict result,
                             Parser *restrict parser) {

    // an empty tuple type is equivalent to a nil type.
    switch (expect(parser, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS: {
        *result = context_nil_type(parser->context);
        return true;
    }

    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    assert(peek(parser, TOK_BEGIN_PAREN));
    if (!nexttok(parser)) { return false; } // eat '('

    TupleType tuple_type;
    tuple_type_create(&tuple_type);

    bool found_comma = false;
    do {
        Type const *element = NULL;
        if (!parse_type(&element, parser)) { return false; }
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
        return error(parser, ERROR_PARSER_EXPECTED_END_PAREN);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    // a tuple type of length 1 is equivalent to that type.
    if (tuple_type.length == 1) {
        *result = tuple_type.types[0];
        tuple_type_destroy(&tuple_type);
    } else {
        *result = context_tuple_type(parser->context, tuple_type);
    }

    return true;
}

static bool parse_type(Type const **restrict result, Parser *restrict parser) {

    switch (parser->curtok) {
    // composite types
    case TOK_BEGIN_PAREN: return parse_tuple_type(result, parser);

    // scalar types
    case TOK_NIL:       *result = context_nil_type(parser->context); break;
    case TOK_TYPE_NIL:  *result = context_nil_type(parser->context); break;
    case TOK_TYPE_BOOL: *result = context_bool_type(parser->context); break;
    case TOK_TYPE_I64:  *result = context_i64_type(parser->context); break;

    default: return error(parser, ERROR_PARSER_EXPECTED_TYPE);
    }

    if (!nexttok(parser)) { return false; } // eat scalar-type
    return true;
}

// formal-argument = identifier ":" type
static bool parse_formal_argument(Local *restrict arg,
                                  Parser *restrict parser) {

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    ConstantString *name = context_intern(parser->context, curtxt(parser));
    if (!nexttok(parser)) { return false; }

    switch (expect(parser, TOK_COLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, ERROR_PARSER_EXPECTED_COLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    Type const *type = NULL;
    if (!parse_type(&type, parser)) { return false; }
    assert(type != NULL);

    arg->name = constant_string_to_view(name);
    arg->type = type;
    return true;
}

// formal-argument-list = "(" (formal-argument ("," formal-argument)*)? ")"
static bool parse_formal_argument_list(Function *restrict body,
                                       Parser *restrict parser) {
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
        return error(parser, ERROR_PARSER_EXPECTED_BEGIN_PAREN);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    switch (expect(parser, TOK_END_PAREN)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        bool comma_found = false;
        do {
            u32    ssa = function_declare_argument(body);
            Local *arg = function_lookup_local(body, ssa);

            if (!parse_formal_argument(arg, parser)) { return false; }

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
            return error(parser, ERROR_PARSER_EXPECTED_END_PAREN);
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
static bool return_(Operand *restrict result, Parser *restrict parser) {

    if (!nexttok(parser)) { return false; } // eat "return"

    if (!expression(result, parser)) { return false; }

    switch (expect(parser, TOK_SEMICOLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    parser_emit_return(parser, *result);
    return true;
}

// constant = "const" identifier "=" expression ";"
static bool let(Operand *restrict result, Parser *restrict parser) {
    exp_assert_debug(peek(parser, TOK_LET));
    if (!nexttok(parser)) { return false; } // eat 'let'

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }
    StringView name = constant_string_to_view(
        context_intern(parser->context, curtxt(parser)));
    if (!nexttok(parser)) { return false; }

    switch (expect(parser, TOK_EQUAL)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, ERROR_PARSER_EXPECTED_EQUAL);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    if (!expression(result, parser)) { return false; }

    switch (expect(parser, TOK_SEMICOLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    *result      = parser_emit_let(parser, *result);
    Local *local = function_lookup_local(parser->function, result->data.ssa);
    local->name  = name;

    return true;
}

// statement = return
//           | let
//           | expression
static bool statement(Operand *restrict result, Parser *restrict parser) {

    switch (parser->curtok) {
    case TOK_RETURN: return return_(result, parser);
    case TOK_LET:    return let(result, parser);

    default: {
        if (!expression(result, parser)) { return false; }

        switch (expect(parser, TOK_SEMICOLON)) {
        case EXPECT_RESULT_SUCCESS: break;
        case EXPECT_RESULT_TOKEN_NOT_FOUND:
            return error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
        case EXPECT_RESULT_FAILURE: return false;
        default:                    EXP_UNREACHABLE();
        }

        return true;
    }
    }
}

static bool parse_block(Operand *restrict result, Parser *restrict parser) {

    switch (expect(parser, TOK_BEGIN_BRACE)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, ERROR_PARSER_EXPECTED_BEGIN_BRACE);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    switch (expect(parser, TOK_END_BRACE)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: {
        bool found_end_brace = false;
        while (!found_end_brace) {
            if (!statement(result, parser)) { return false; }
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

static bool function(Operand *restrict result, Parser *restrict parser) {
    exp_assert_debug(peek(parser, TOK_FN));
    if (!nexttok(parser)) { return false; } // eat "fn"

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    StringView name = constant_string_to_view(
        context_intern(parser->context, curtxt(parser)));
    if (!nexttok(parser)) { return false; }

    Value    *value    = value_allocate_function();
    Function *function = &value->function;

    Function *previous = parser->function;
    parser->function   = function;

    if (!parse_formal_argument_list(function, parser)) { return false; }

    switch (expect(parser, TOK_RIGHT_ARROW)) {
    case EXPECT_RESULT_SUCCESS:
        if (!parse_type(&function->return_type, parser)) { return false; }
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    if (!parse_block(result, parser)) { return false; }

    parser->function = previous;
    Value const *fn  = context_constant_function(parser->context, value);
    *result          = parser_emit_let(parser, operand_constant(fn));
    exp_assert_debug(result->kind == OPERAND_KIND_SSA);
    Local *local = function_lookup_local(parser->function, result->data.ssa);
    local->name  = name;

    return true;
}

static bool lambda(Operand *restrict result, Parser *restrict parser) {
    exp_assert_debug(peek(parser, TOK_BACKSLASH));
    if (!nexttok(parser)) { return false; } // eat "\"

    Value    *value    = value_allocate_function();
    Function *function = &value->function;

    Function *previous = parser->function;
    parser->function   = function;

    if (!parse_formal_argument_list(function, parser)) { return false; }

    switch (expect(parser, TOK_RIGHT_ARROW)) {
    case EXPECT_RESULT_SUCCESS:
        if (!parse_type(&function->return_type, parser)) { return false; }
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    if (!parse_block(result, parser)) { return false; }

    parser->function = previous;
    Value const *fn  = context_constant_function(parser->context, value);
    *result          = operand_constant(fn);
    return true;
}

static bool parse_tuple(Tuple *restrict tuple, Parser *restrict parser) {
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
            if (!expression(&result, parser)) { return false; }
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
            return error(parser, ERROR_PARSER_EXPECTED_END_PAREN);
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

static bool parens(Operand *restrict result, Parser *restrict parser) {
    Value *value = value_allocate_tuple();
    Tuple *tuple = &value->tuple;

    if (!parse_tuple(tuple, parser)) { return false; };

    if (tuple->length == 0) {
        *result = operand_constant(context_constant_nil(parser->context));
        value_deallocate(value);
    } else if (tuple->length == 1) {
        *result = tuple->elements[0];
        value_deallocate(value);
    } else {
        PANIC("#TODO: Add support for Tuples");
        //*result =
        //    operand_constant(context_constant_tuple(parser->context, value));
    }

    return true;
}

static bool unop(Operand *restrict result, Parser *restrict parser) {
    Token op = parser->curtok;
    if (!nexttok(parser)) { return false; }

    if (!parse_precedence(result, PREC_UNARY, parser)) { return false; }

    switch (op) {
    case TOK_MINUS: *result = parser_emit_neg(parser, *result); break;
    default:        EXP_UNREACHABLE();
    }
    return true;
}

static bool
binop(Operand *restrict result, Operand left, Parser *restrict parser) {
    Token      op   = parser->curtok;
    ParseRule *rule = get_rule(op);
    if (!nexttok(parser)) { return false; } // eat the operator

    // parse the right hand side
    if (!parse_precedence(result, (Precedence)(rule->precedence + 1), parser)) {
        return false;
    }
    Operand right = *result;

    switch (op) {
    case TOK_DOT:     *result = parser_emit_dot(parser, left, right); break;
    case TOK_PLUS:    *result = parser_emit_add(parser, left, right); break;
    case TOK_MINUS:   *result = parser_emit_sub(parser, left, right); break;
    case TOK_STAR:    *result = parser_emit_mul(parser, left, right); break;
    case TOK_SLASH:   *result = parser_emit_div(parser, left, right); break;
    case TOK_PERCENT: *result = parser_emit_mod(parser, left, right); break;

    default: EXP_UNREACHABLE();
    }
    return true;
}

static bool
call(Operand *restrict result, Operand left, Parser *restrict parser) {
    Value *value         = value_allocate_tuple();
    Tuple *argument_list = &value->tuple;

    if (!parse_tuple(argument_list, parser)) { return false; }

    Value const *tuple = context_constant_tuple(parser->context, value);
    Operand      actual_arguments = operand_constant(tuple);

    *result = parser_emit_call(parser, left, actual_arguments);
    return true;
}

static bool nil(Operand *restrict result, Parser *restrict parser) {
    assert(peek(parser, TOK_NIL));
    if (!nexttok(parser)) { return false; }
    *result = operand_nil();
    return true;
}

static bool boolean_true(Operand *restrict result, Parser *restrict parser) {
    assert(peek(parser, TOK_TRUE));
    if (!nexttok(parser)) { return false; }
    *result = operand_bool(true);
    return true;
}

static bool boolean_false(Operand *restrict result, Parser *restrict parser) {
    assert(peek(parser, TOK_FALSE));
    if (!nexttok(parser)) { return false; }
    *result = operand_bool(false);
    return true;
}

static bool integer(Operand *restrict result, Parser *restrict parser) {
    assert(peek(parser, TOK_INTEGER));
    StringView sv      = curtxt(parser);
    u64        integer = 0;

    if (!str_to_u64(&integer, sv.ptr, sv.length)) {
        return error(parser, ERROR_PARSER_INTEGER_LITERAL_OUT_OF_RANGE);
    }

    if (!nexttok(parser)) { return false; }
    if (u64_in_range_i64(integer)) {
        *result = operand_i64((i64)integer);
    } else {
        *result = operand_u64(integer);
    }

    return true;
}

static bool identifier(Operand *restrict result, Parser *restrict parser) {
    exp_assert_debug(peek(parser, TOK_IDENTIFIER));
    ConstantString *name = context_intern(parser->context, curtxt(parser));
    if (!nexttok(parser)) { return false; }
    // We could perform lookup here, but that forces the source code to provide
    // definitions before usage, precluding mutually recursive Functions and
    // Types. This is an active area of development. For now we delay the
    // generation of the error till typechecking.
    *result = operand_label(name);
    return true;
}

static bool expression(Operand *restrict result, Parser *restrict parser) {
    return parse_precedence(result, PREC_ASSIGNMENT, parser);
}

static bool parse_precedence(Operand *restrict result,
                             Precedence precedence,
                             Parser *restrict parser) {
    ParseRule *rule = get_rule(parser->curtok);
    if (rule->prefix == NULL) {
        return error(parser, ERROR_PARSER_EXPECTED_EXPRESSION);
    }
    // Parse the left hand side of the expression
    if (!rule->prefix(result, parser)) { return false; }

    while (1) {

        ParseRule *rule = get_rule(parser->curtok);

        if (precedence > rule->precedence) { break; }

        Operand left;
        if (!rule->infix(&left, *result, parser)) { return false; }

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
        [TOK_END_BRACE]   = {         NULL,  NULL,   PREC_NONE},
        [TOK_DOT]         = {         NULL, binop,   PREC_CALL},
        [TOK_COMMA]       = {         NULL,  NULL,   PREC_NONE},
        [TOK_SEMICOLON]   = {         NULL,  NULL,   PREC_NONE},
        [TOK_COLON]       = {         NULL,  NULL,   PREC_NONE},
        [TOK_RIGHT_ARROW] = {         NULL,  NULL,   PREC_NONE},
        [TOK_BACKSLASH]   = {       lambda,  NULL,   PREC_CALL},

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
        [TOK_TYPE_U8]   = {         NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_U16]  = {         NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_U32]  = {         NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_U64]  = {         NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_I8]   = {         NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_I16]  = {         NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_I32]  = {         NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_I64]  = {         NULL,  NULL,   PREC_NONE},
    };

    return &rules[token];
}

static bool top_level_expression(Parser *restrict parser) {
    Operand result;
    switch (parser->curtok) {
    case TOK_LET: return let(&result, parser);
    case TOK_FN:  return function(&result, parser);

    default: return error(parser, ERROR_PARSER_EXPECTED_TOP_LEVEL_DECLARATION);
    }
}

bool parser_parse_expression(Parser *restrict parser,
                             Function *restrict expression) {
    assert(parser != NULL);
    assert(expression != NULL);
    parser->function = expression;
    return top_level_expression(parser);
}
