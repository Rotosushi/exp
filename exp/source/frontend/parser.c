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
    Context *context;
    FunctionBody *function;
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

typedef bool (*PrefixFunction)(Operand *result, Parser *parser);
typedef bool (*InfixFunction)(Operand *result, Operand left, Parser *parser);

typedef struct ParseRule {
    PrefixFunction prefix;
    InfixFunction infix;
    Precedence precedence;
} ParseRule;

static void parser_initialize(Parser *parser, Context *context) {
    assert(parser != nullptr);
    assert(context != nullptr);
    parser->lexer    = lexer_create();
    parser->curtok   = TOK_END;
    parser->context  = context;
    parser->function = nullptr;
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

static bool error(Parser *parser, ErrorCode code) {
    Error *current_error = context_current_error(parser->context);
    error_assign(current_error, code, lexer_current_text(&parser->lexer));
    return false;
}

// static u64 curcol(Parser * parser) {
//   return lexer_current_column(&parser->lexer);
// }

static bool peek(Parser *parser, Token token) {
    return parser->curtok == token;
}

static bool comment(Parser *parser) {
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
            if (!comment(parser)) { return false; }
        }
    }

    // eat '*/'
    parser->curtok = lexer_scan(&parser->lexer);
    return true;
}

static bool nexttok(Parser *parser) {
    if (lexer_at_end(&parser->lexer)) {
        parser->curtok = TOK_END;
        return true;
    }

    parser->curtok = lexer_scan(&parser->lexer);

    while (parser->curtok == TOK_BEGIN_COMMENT) {
        if (!comment(parser)) { return false; }
    }

    return true;
}

typedef enum ExpectResult {
    EXPECT_RESULT_SUCCESS,
    EXPECT_RESULT_TOKEN_NOT_FOUND,
    EXPECT_RESULT_FAILURE,
} ExpectResult;

static ExpectResult expect(Parser *parser, Token token) {
    if (!peek(parser, token)) { return EXPECT_RESULT_TOKEN_NOT_FOUND; }
    if (!nexttok(parser)) { return EXPECT_RESULT_FAILURE; }
    return EXPECT_RESULT_SUCCESS;
}

static ParseRule *get_rule(Token token);
static bool expression(Operand *result, Parser *parser);
static bool
parse_precedence(Operand *result, Precedence precedence, Parser *parser);

static bool parse_type(Type const **result, Parser *parser);

static bool parse_tuple_type(Type const **result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
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
    tuple_type_initialize(&tuple_type);

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
    if (tuple_type.count == 1) {
        *result = tuple_type.types[0];
        tuple_type_terminate(&tuple_type);
    } else {
        *result = context_tuple_type(parser->context, tuple_type);
    }

    return true;
}

static bool parse_type(Type const **result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    switch (parser->curtok) {
    // composite types
    case TOK_BEGIN_PAREN: return parse_tuple_type(result, parser);

    // scalar types
    case TOK_NIL:       *result = context_nil_type(parser->context); break;
    case TOK_TYPE_NIL:  *result = context_nil_type(parser->context); break;
    case TOK_TYPE_BOOL: *result = context_boolean_type(parser->context); break;
    case TOK_TYPE_I64:  *result = context_i32_type(parser->context); break;

    default: return error(parser, ERROR_PARSER_EXPECTED_TYPE);
    }

    if (!nexttok(parser)) { return false; } // eat scalar-type
    return true;
}

// formal-argument = identifier ":" type
static bool parse_formal_argument(FormalArgument *arg, Parser *parser) {
    assert(arg != nullptr);
    assert(parser != nullptr);
    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    StringView name = context_intern(parser->context, curtxt(parser));
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

    arg->name = name;
    arg->type = type;
    return true;
}

// formal-argument-list = "(" (formal-argument ("," formal-argument)*)? ")"
static bool parse_formal_argument_list(Parser *parser) {
    assert(parser != nullptr);
    assert(parser->function != nullptr);
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
            FormalArgument arg = {};
            if (!parse_formal_argument(&arg, parser)) { return false; }
            function_body_allocate_argument(parser->function, arg);

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
static bool return_(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->function != nullptr);
    if (!nexttok(parser)) { return false; } // eat "return"

    if (!expression(result, parser)) { return false; }

    switch (expect(parser, TOK_SEMICOLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    function_body_append_instruction(parser->function,
                                     instruction_return(*result));
    return true;
}

// constant = "const" identifier "=" expression ";"
static bool constant(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->context != nullptr);
    if (!nexttok(parser)) { return false; } // eat 'const'

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }
    StringView name = context_intern(parser->context, curtxt(parser));
    Operand label   = context_labels_insert(parser->context, name);
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

    u32 ssa      = function_body_declare_local(parser->function);
    Local *local = function_body_local_at(parser->function, ssa);
    local_update_label(local, name);

    function_body_append_instruction(parser->function,
                                     instruction_load(label, *result));
    return true;
}

// statement = return
//           | constant
//           | expression
static bool statement(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    switch (parser->curtok) {
    case TOK_RETURN: return return_(result, parser);
    case TOK_CONST:  return constant(result, parser);

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

static bool parse_block(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
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

static bool function(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(peek(parser, TOK_FN));
    if (!nexttok(parser)) { return false; } // eat "fn"

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }

    StringView name = context_intern(parser->context, curtxt(parser));
    if (!nexttok(parser)) { return false; }

    Symbol *symbol   = context_symbol_table_at(parser->context, name);
    parser->function = &symbol->function_body;

    if (!parse_formal_argument_list(parser)) { return false; }

    switch (expect(parser, TOK_RIGHT_ARROW)) {
    case EXPECT_RESULT_SUCCESS:
        if (!parse_type(&parser->function->return_type, parser)) {
            return false;
        }
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    if (!parse_block(result, parser)) { return false; }

#ifndef NDEBUG
    file_write("parsed a function: \nfn ", stdout);
    print_string_view(name, stdout);
    String buffer;
    string_initialize(&buffer);
    print_function_body(&buffer, parser->function, parser->context);
    file_write(string_to_cstring(&buffer), stdout);
    string_destroy(&buffer);
    file_write("\n", stdout);
#endif

    parser->function = nullptr;
    return true;
}

static bool definition(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    switch (parser->curtok) {
    case TOK_FN: return function(result, parser);

    default: return error(parser, ERROR_PARSER_EXPECTED_KEYWORD_FN);
    }
}

static bool parse_tuple(Tuple *tuple, Parser *parser) {
    assert(tuple != nullptr);
    assert(parser != nullptr);
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

static bool parens(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    Tuple tuple = tuple_create();

    if (!parse_tuple(&tuple, parser)) { return false; };

    if (tuple.size == 0) {
        *result = context_constants_append(parser->context, value_create_nil());
        tuple_destroy(&tuple);
    } else if (tuple.size == 1) {
        *result = tuple.elements[0];
        tuple_destroy(&tuple);
    } else {
        *result = context_constants_append(parser->context,
                                           value_create_tuple(tuple));
    }

    return true;
}

static bool unop(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->function != nullptr);
    Token op = parser->curtok;
    if (!nexttok(parser)) { return false; }

    Operand right;
    if (!parse_precedence(&right, PREC_UNARY, parser)) { return false; }

    switch (op) {
    case TOK_MINUS: {
        *result = operand_ssa(function_body_declare_local(parser->function));
        function_body_append_instruction(parser->function,
                                         instruction_negate(*result, right));
        break;
    }
    default: EXP_UNREACHABLE();
    }
    return true;
}

static bool binop(Operand *result, Operand left, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->function != nullptr);

    Token op        = parser->curtok;
    ParseRule *rule = get_rule(op);
    if (!nexttok(parser)) { return false; } // eat the operator

    Operand right;
    if (!parse_precedence(&right, (Precedence)(rule->precedence + 1), parser)) {
        return false;
    }

    switch (op) {
    case TOK_DOT: {
        *result = operand_ssa(function_body_declare_local(parser->function));
        function_body_append_instruction(parser->function,
                                         instruction_dot(*result, left, right));
        break;
    }

    case TOK_PLUS: {
        *result = operand_ssa(function_body_declare_local(parser->function));
        function_body_append_instruction(parser->function,
                                         instruction_add(*result, left, right));
        break;
    }

    case TOK_MINUS: {
        *result = operand_ssa(function_body_declare_local(parser->function));
        function_body_append_instruction(
            parser->function, instruction_subtract(*result, left, right));
        break;
    }

    case TOK_STAR: {
        *result = operand_ssa(function_body_declare_local(parser->function));
        function_body_append_instruction(
            parser->function, instruction_multiply(*result, left, right));
        break;
    }

    case TOK_SLASH: {
        *result = operand_ssa(function_body_declare_local(parser->function));
        function_body_append_instruction(
            parser->function, instruction_divide(*result, left, right));
        break;
    }

    case TOK_PERCENT: {
        *result = operand_ssa(function_body_declare_local(parser->function));
        function_body_append_instruction(
            parser->function, instruction_modulus(*result, left, right));
        break;
    }

    default: EXP_UNREACHABLE();
    }
    return true;
}

static bool call(Operand *result, Operand left, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->context != nullptr);
    assert(parser->function != nullptr);
    Tuple argument_list = tuple_create();

    if (!parse_tuple(&argument_list, parser)) { return false; }

    Operand right = context_constants_append(parser->context,
                                             value_create_tuple(argument_list));

    *result = operand_ssa(function_body_declare_local(parser->function));
    function_body_append_instruction(parser->function,
                                     instruction_call(*result, left, right));
    return true;
}

static bool nil(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->context != nullptr);
    assert(peek(parser, TOK_NIL));
    if (!nexttok(parser)) { return false; }
    *result = context_constants_append(parser->context, value_create_nil());
    return true;
}

static bool boolean_true(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->context != nullptr);
    assert(peek(parser, TOK_TRUE));
    if (!nexttok(parser)) { return false; }
    *result =
        context_constants_append(parser->context, value_create_boolean(true));
    return true;
}

static bool boolean_false(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->context != nullptr);
    assert(peek(parser, TOK_FALSE));
    if (!nexttok(parser)) { return false; }
    *result =
        context_constants_append(parser->context, value_create_boolean(0));
    return true;
}

static bool integer(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->context != nullptr);
    assert(peek(parser, TOK_INTEGER));
    StringView sv = curtxt(parser);
    i64 value     = str_to_i64(sv.ptr, sv.length);

    if (!nexttok(parser)) { return false; }
    if (i64_in_range_i32(value)) {
        *result = operand_i32((i32)value);
    } else {
        return error(parser, ERROR_INTEGER_TO_LARGE);
    }

    return true;
}

static bool identifier(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    assert(parser->context != nullptr);
    assert(parser->function != nullptr);
    assert(peek(parser, TOK_IDENTIFIER));
    StringView name = context_intern(parser->context, curtxt(parser));
    *result         = context_labels_insert(parser->context, name);
    if (!nexttok(parser)) { return false; }
    return true;
}

static bool expression(Operand *result, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
    return parse_precedence(result, PREC_ASSIGNMENT, parser);
}

static bool
parse_precedence(Operand *result, Precedence precedence, Parser *parser) {
    assert(result != nullptr);
    assert(parser != nullptr);
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

i32 parse_buffer(char const *buffer, u64 length, Context *context) {
    assert(buffer != NULL);
    assert(context != NULL);

    Parser parser;
    parser_initialize(&parser, context);
    parser_set_view(&parser, buffer, length);

    if (!nexttok(&parser)) {
        error_print(context_current_error(context),
                    context_source_path(context),
                    curline(&parser));
        return EXIT_FAILURE;
    }

    while (!finished(&parser)) {
        Operand result;
        if (!definition(&result, &parser)) {
            error_print(context_current_error(context),
                        context_source_path(context),
                        curline(&parser));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

i32 parse_source(Context *context) {
    assert(context != NULL);
    StringView path = context_source_path(context);
    FILE *file      = file_open(path.ptr, "r");
    String buffer;
    string_from_file(&buffer, file);
    file_close(file);
    i32 result =
        parse_buffer(string_to_cstring(&buffer), buffer.length, context);
    string_destroy(&buffer);
    return result;
}
