/**
 * Copyright (C) 2025 Cade Weinberg
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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file frontend/parser.h
 */
#include "frontend/parser.h"
#include "env/error.h"
#include "frontend/lexer.h"
#include "frontend/token.h"
#include "imr/operand.h"
#include "imr/scalar.h"
#include "imr/value.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/assert.h"
#include "utility/numeric_conversions.h"
#include "utility/unreachable.h"

typedef struct Parser {
    Lexer lexer;
    Token curtok;
    Context *context;
    Function *function;
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

/**
 * @brief
 */
typedef bool (*PrefixFunction)(Operand *result, Parser *parser);
typedef bool (*InfixFunction)(Operand *result, Operand left, Parser *parser);

typedef struct ParseRule {
    PrefixFunction prefix;
    InfixFunction infix;
    Precedence precedence;
} ParseRule;

static void parser_initialize(Parser *parser, Context *context) {
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(context != nullptr);
    parser->lexer    = lexer_create();
    parser->curtok   = TOK_END;
    parser->context  = context;
    parser->function = nullptr;
}

static void parser_set_view(Parser *parser, char const *buffer, u64 length) {
    EXP_ASSERT(parser != NULL);
    EXP_ASSERT(buffer != NULL);
    lexer_set_view(&(parser->lexer), buffer, length);
}

static bool finished(Parser *parser) {
    EXP_ASSERT(parser != nullptr);
    return parser->curtok == TOK_END;
}

static StringView curtxt(Parser *parser) {
    EXP_ASSERT(parser != nullptr);
    return lexer_current_text(&parser->lexer);
}

static u64 curline(Parser *parser) {
    EXP_ASSERT(parser != nullptr);
    return lexer_current_line(&parser->lexer);
}

static bool error(Parser *parser, ErrorCode code) {
    EXP_ASSERT(parser != nullptr);
    Error *current_error = context_current_error(parser->context);
    error_assign(current_error, code, lexer_current_text(&parser->lexer));
    return false;
}

// static u64 curcol(Parser * parser) {
//   return lexer_current_column(&parser->lexer);
// }

static bool peek(Parser *parser, Token token) {
    EXP_ASSERT(parser != nullptr);
    return parser->curtok == token;
}

static bool comment(Parser *parser) {
    EXP_ASSERT(parser != nullptr);
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
    EXP_ASSERT(parser != nullptr);
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
    EXP_ASSERT(parser != nullptr);
    if (!peek(parser, token)) { return EXPECT_RESULT_TOKEN_NOT_FOUND; }
    if (!nexttok(parser)) { return EXPECT_RESULT_FAILURE; }
    return EXPECT_RESULT_SUCCESS;
}

static ParseRule *get_rule(Token token);
static bool expression(Operand *result, Parser *parser);
static bool parse_precedence(Operand *result, Precedence precedence,
                             Parser *parser);

static bool parse_type(Type const **result, Parser *parser);

static bool parse_tuple_type(Type const **result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(peek(parser, TOK_BEGIN_PAREN));
    if (!nexttok(parser)) { return false; } // eat '('

    TupleType tuple_type;
    tuple_type_initialize(&tuple_type);
    bool found_comma = false;
    do {
        Type const *element = NULL;
        if (!parse_type(&element, parser)) { return false; }
        EXP_ASSERT(element != NULL);

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
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    switch (parser->curtok) {
    // composite types
    case TOK_BEGIN_PAREN: return parse_tuple_type(result, parser);

    // scalar types
    case TOK_TYPE_NIL:  *result = context_nil_type(parser->context); break;
    case TOK_TYPE_BOOL: *result = context_bool_type(parser->context); break;
    case TOK_TYPE_I8:   *result = context_i8_type(parser->context); break;
    case TOK_TYPE_I16:  *result = context_i16_type(parser->context); break;
    case TOK_TYPE_I32:  *result = context_i32_type(parser->context); break;
    case TOK_TYPE_I64:  *result = context_i64_type(parser->context); break;
    case TOK_TYPE_U8:   *result = context_u8_type(parser->context); break;
    case TOK_TYPE_U16:  *result = context_u16_type(parser->context); break;
    case TOK_TYPE_U32:  *result = context_u32_type(parser->context); break;
    case TOK_TYPE_U64:  *result = context_u64_type(parser->context); break;

    default: return error(parser, ERROR_PARSER_EXPECTED_TYPE);
    }

    if (!nexttok(parser)) { return false; } // eat scalar-type
    return true;
}

// formal-argument = identifier ":" type
static bool parse_formal_argument(FormalArgument *arg, Parser *parser) {
    EXP_ASSERT(arg != nullptr);
    EXP_ASSERT(parser != nullptr);
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
    EXP_ASSERT(type != NULL);

    arg->type = type;
    arg->name = name;
    return true;
}

// formal-argument-list = "(" (formal-argument ("," formal-argument)*)? ")"
static bool parse_formal_argument_list(Parser *parser) {
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->function != nullptr);
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
        // u8 index         = 0;
        bool comma_found = false;
        do {
            FormalArgument argument = {};
            if (!parse_formal_argument(&argument, parser)) { return false; }
            function_arguments_append(parser->function, argument);

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
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->function != nullptr);
    EXP_ASSERT(peek(parser, TOK_RETURN));
    if (!nexttok(parser)) { return false; } // eat "return"

    if (!expression(result, parser)) { return false; }

    switch (expect(parser, TOK_SEMICOLON)) {
    case EXPECT_RESULT_SUCCESS: break;
    case EXPECT_RESULT_TOKEN_NOT_FOUND:
        return error(parser, ERROR_PARSER_EXPECTED_SEMICOLON);
    case EXPECT_RESULT_FAILURE: return false;
    default:                    EXP_UNREACHABLE();
    }

    function_append_return(parser->function, parser->context, *result);
    return true;
}

// constant = "const" identifier "=" expression ";"
static bool constant(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    if (!nexttok(parser)) { return false; } // eat 'const'

    if (!peek(parser, TOK_IDENTIFIER)) {
        return error(parser, ERROR_PARSER_EXPECTED_IDENTIFIER);
    }
    StringView name = context_intern(parser->context, curtxt(parser));
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

    u32 ssa      = function_declare_local(parser->function);
    Local *local = function_local_at(parser->function, ssa);
    local_update_label(local, name);
    function_append_instruction(parser->function,
                                instruction_load(operand_ssa(ssa), *result));
    return true;
}

// statement = return
//           | constant
//           | expression
static bool statement(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
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
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
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
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(peek(parser, TOK_FN));
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

    /*
#ifndef NDEBUG
    file_write("parsed a function: \nfn ", stdout);
    file_write(name->buffer, stdout);
    String buffer;
    string_initialize(&buffer);
    print_function(&buffer, parser->function, parser->context);
    file_write(string_to_cstring(&buffer), stdout);
    string_terminate(&buffer);
    file_write("\n", stdout);
#endif
*/

    parser->function = nullptr;
    return true;
}

static bool definition(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    switch (parser->curtok) {
    case TOK_FN: return function(result, parser);

    default: return error(parser, ERROR_PARSER_EXPECTED_KEYWORD_FN);
    }
}

static bool parse_tuple(Tuple *tuple, Parser *parser) {
    EXP_ASSERT(tuple != nullptr);
    EXP_ASSERT(parser != nullptr);
    switch (expect(parser, TOK_NIL)) {
    case EXPECT_RESULT_SUCCESS:         return true;
    case EXPECT_RESULT_TOKEN_NOT_FOUND: break;
    case EXPECT_RESULT_FAILURE:         return false;
    default:                            EXP_UNREACHABLE();
    }

    EXP_ASSERT(peek(parser, TOK_BEGIN_PAREN));
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
                tuple_terminate(tuple);
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
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    Tuple tuple;
    tuple_initialize(&tuple);

    if (!parse_tuple(&tuple, parser)) { return false; };

    if (tuple.length == 0) {
        *result = operand_constant(
            context_constants_append_tuple(parser->context, tuple));
    } else if (tuple.length == 1) {
        *result = tuple.elements[0];
        tuple_terminate(&tuple);
    } else {
        *result = operand_constant(
            context_constants_append_tuple(parser->context, tuple));
    }

    return true;
}

static bool unop(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->function != nullptr);
    Token op = parser->curtok;
    if (!nexttok(parser)) { return false; }

    Operand right;
    if (!parse_precedence(&right, PREC_UNARY, parser)) { return false; }

    switch (op) {
    case TOK_MINUS: {
        *result = operand_ssa(function_declare_local(parser->function));
        function_append_instruction(parser->function,
                                    instruction_neg(*result, right));
        break;
    }
    default: EXP_UNREACHABLE();
    }
    return true;
}

static bool binop(Operand *result, Operand left, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->function != nullptr);

    Token op        = parser->curtok;
    ParseRule *rule = get_rule(op);
    if (!nexttok(parser)) { return false; } // eat the operator

    Operand right;
    if (!parse_precedence(&right, (Precedence)(rule->precedence + 1), parser)) {
        return false;
    }

    switch (op) {
    case TOK_DOT: {
        *result = operand_ssa(function_declare_local(parser->function));
        function_append_instruction(parser->function,
                                    instruction_dot(*result, left, right));
        break;
    }

    case TOK_PLUS: {
        *result = operand_ssa(function_declare_local(parser->function));
        function_append_instruction(parser->function,
                                    instruction_add(*result, left, right));
        break;
    }

    case TOK_MINUS: {
        *result = operand_ssa(function_declare_local(parser->function));
        function_append_instruction(parser->function,
                                    instruction_sub(*result, left, right));
        break;
    }

    case TOK_STAR: {
        *result = operand_ssa(function_declare_local(parser->function));
        function_append_instruction(parser->function,
                                    instruction_mul(*result, left, right));
        break;
    }

    case TOK_SLASH: {
        *result = operand_ssa(function_declare_local(parser->function));
        function_append_instruction(parser->function,
                                    instruction_div(*result, left, right));
        break;
    }

    case TOK_PERCENT: {
        *result = operand_ssa(function_declare_local(parser->function));
        function_append_instruction(parser->function,
                                    instruction_mod(*result, left, right));
        break;
    }

    default: EXP_UNREACHABLE();
    }
    return true;
}

static bool call(Operand *result, Operand left, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    EXP_ASSERT(parser->function != nullptr);
    Tuple argument_list;
    tuple_initialize(&argument_list);

    if (!parse_tuple(&argument_list, parser)) { return false; }

    Operand right = operand_constant(
        context_constants_append_tuple(parser->context, argument_list));

    *result = operand_ssa(function_declare_local(parser->function));
    function_append_instruction(parser->function,
                                instruction_call(*result, left, right));
    return true;
}

static bool nil(Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    EXP_ASSERT(peek(parser, TOK_NIL));
    if (!nexttok(parser)) { return false; }
    Value result;
    value_initialize_scalar(&result, scalar_nil());
    context_stack_push(parser->context, result);
    return true;
}

static bool boolean_true(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    EXP_ASSERT(peek(parser, TOK_TRUE));
    if (!nexttok(parser)) { return false; }
    Value result;
    value_initialize_scalar(&result, scalar_bool(true));
    context_stack_push(parser->context, result);
    return true;
}

static bool boolean_false(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    EXP_ASSERT(peek(parser, TOK_FALSE));
    if (!nexttok(parser)) { return false; }
    Value result;
    value_initialize_scalar(&result, scalar_bool(false));
    context_stack_push(parser->context, result);
    return true;
}

static bool integer_annotated(Parser *parser) {
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    EXP_ASSERT(peek(parser, TOK_COLON));
    if (!nexttok(parser)) { return false; }
    Scalar scalar = scalar_uninitialized();
    switch (parser->curtok) {
    case TOK_TYPE_I8: {
        if (u64_in_range_i8(value)) {
            scalar = scalar_i8((i8)value);
        } else {
            return error(parser, ERROR_PARSER_INTEGER_OUT_OF_RANGE);
        }
        break;
    }

    case TOK_TYPE_I16: {
        if (u64_in_range_i16(value)) {
            scalar = scalar_i16((i16)value);
        } else {
            return error(parser, ERROR_PARSER_INTEGER_OUT_OF_RANGE);
        }
        break;
    }

    case TOK_TYPE_I32: {
        if (u64_in_range_i32(value)) {
            scalar = scalar_i32((i32)value);
        } else {
            return error(parser, ERROR_PARSER_INTEGER_OUT_OF_RANGE);
        }
        break;
    }

    case TOK_TYPE_I64: {
        if (u64_in_range_i64(value)) {
            scalar = scalar_i64((i64)value);
        } else {
            return error(parser, ERROR_PARSER_INTEGER_OUT_OF_RANGE);
        }
        break;
    }

    case TOK_TYPE_U8: {
        if (u64_in_range_u8(value)) {
            scalar = scalar_u8((u8)value);
        } else {
            return error(parser, ERROR_PARSER_INTEGER_OUT_OF_RANGE);
        }
        break;
    }

    case TOK_TYPE_U16: {
        if (u64_in_range_u16(value)) {
            scalar = scalar_u16((u16)value);
        } else {
            return error(parser, ERROR_PARSER_INTEGER_OUT_OF_RANGE);
        }
        break;
    }

    case TOK_TYPE_U32: {
        if (u64_in_range_u32(value)) {
            scalar = scalar_u32((u32)value);
        } else {
            return error(parser, ERROR_PARSER_INTEGER_OUT_OF_RANGE);
        }
        break;
    }

    default: return error(parser, ERROR_PARSER_EXPECTED_INTEGER_TYPE);
    }
    Value result;
    value_initialize_scalar(&result, scalar);
    context_stack_push(parser->context, result);
    return true;
}

static bool integer(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    EXP_ASSERT(peek(parser, TOK_INTEGER));
    StringView sv = curtxt(parser);
    u64 value     = str_to_u64(sv.ptr, sv.length);
    // eat the integer literal
    if (!nexttok(parser)) { return false; }

    // check if the user provided an integer type annotation.
    if (peek(parser, TOK_COLON)) { return integer_annotated(parser); }

    // @note the existence of a user provided type annotation telling
    // the compiler what type to use for the integer literal gives me
    // the thought of a type annotation to a string literal,
    // allowing the user to construct a custom type out of that string,
    // by way of a custom parsing function provided by the user.

    Scalar scalar = scalar_uninitialized();
    if (u64_in_range_i64(value)) {
        scalar = scalar_i64((i64)value);
    } else {
        scalar = scalar_u64(value);
    }

    Value result;
    value_initialize_scalar(&result, scalar);
    context_stack_push(parser->context, result);
    return true;
}

static bool identifier(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    EXP_ASSERT(parser->context != nullptr);
    EXP_ASSERT(parser->function != nullptr);
    EXP_ASSERT(peek(parser, TOK_IDENTIFIER));
    StringView name = context_intern(parser->context, curtxt(parser));
    u32 label       = context_labels_append(parser->context, name);
    *result         = operand_label(label);
    if (!nexttok(parser)) { return false; }
    return true;
}

static bool expression(Operand *result, Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
    return parse_precedence(result, PREC_ASSIGNMENT, parser);
}

static bool parse_precedence(Operand *result, Precedence precedence,
                             Parser *parser) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(parser != nullptr);
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
        [TOK_END] = {      NULL,  NULL,   PREC_NONE},

        [TOK_ERROR_UNEXPECTED_CHAR]        = {      NULL,  NULL,   PREC_NONE},
        [TOK_ERROR_UNMATCHED_DOUBLE_QUOTE] = {      NULL,  NULL,   PREC_NONE},

        [TOK_BEGIN_COMMENT] = {      NULL,  NULL,   PREC_NONE},
        [TOK_END_COMMENT]   = {      NULL,  NULL,   PREC_NONE},
        [TOK_BEGIN_PAREN]   = {    parens,  call,   PREC_CALL},
        [TOK_END_PAREN]     = {      NULL,  NULL,   PREC_NONE},
        [TOK_BEGIN_BRACE]   = {      NULL,  NULL,   PREC_NONE},
        [TOK_COMMA]         = {      NULL,  NULL,   PREC_NONE},
        [TOK_DOT]           = {      NULL, binop,   PREC_CALL},
        [TOK_SEMICOLON]     = {      NULL,  NULL,   PREC_NONE},
        [TOK_COLON]         = {      NULL,  NULL,   PREC_NONE},
        [TOK_RIGHT_ARROW]   = {      NULL,  NULL,   PREC_NONE},

        [TOK_MINUS]         = {      unop, binop,   PREC_TERM},
        [TOK_PLUS]          = {      NULL, binop,   PREC_TERM},
        [TOK_SLASH]         = {      NULL, binop, PREC_FACTOR},
        [TOK_STAR]          = {      NULL, binop, PREC_FACTOR},
        [TOK_PERCENT]       = {      NULL, binop, PREC_FACTOR},
        [TOK_BANG]          = {      NULL,  NULL,   PREC_NONE},
        [TOK_BANG_EQUAL]    = {      NULL,  NULL,   PREC_NONE},
        [TOK_EQUAL]         = {      NULL,  NULL,   PREC_NONE},
        [TOK_EQUAL_EQUAL]   = {      NULL,  NULL,   PREC_NONE},
        [TOK_GREATER]       = {      NULL,  NULL,   PREC_NONE},
        [TOK_GREATER_EQUAL] = {      NULL,  NULL,   PREC_NONE},
        [TOK_LESS]          = {      NULL,  NULL,   PREC_NONE},
        [TOK_LESS_EQUAL]    = {      NULL,  NULL,   PREC_NONE},
        [TOK_AND]           = {      NULL,  NULL,   PREC_NONE},
        [TOK_OR]            = {      NULL,  NULL,   PREC_NONE},
        [TOK_XOR]           = {      NULL,  NULL,   PREC_NONE},

        [TOK_FN]     = {      NULL,  NULL,   PREC_NONE},
        [TOK_VAR]    = {      NULL,  NULL,   PREC_NONE},
        [TOK_CONST]  = {      NULL,  NULL,   PREC_NONE},
        [TOK_RETURN] = {      NULL,  NULL,   PREC_NONE},

        [TOK_NIL]            = {      NULL,  call,   PREC_CALL},
        [TOK_TRUE]           = {      NULL,  NULL,   PREC_NONE},
        [TOK_FALSE]          = {      NULL,  NULL,   PREC_NONE},
        [TOK_INTEGER]        = {   integer,  NULL,   PREC_NONE},
        [TOK_STRING_LITERAL] = {      NULL,  NULL,   PREC_NONE},
        [TOK_IDENTIFIER]     = {identifier,  NULL,   PREC_NONE},

        [TOK_TYPE_NIL]  = {      NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_BOOL] = {      NULL,  NULL,   PREC_NONE},
        [TOK_TYPE_I32]  = {      NULL,  NULL,   PREC_NONE},
    };

    return &rules[token];
}

ExpResult parse_expression(StringView source, Context *context) {
    EXP_ASSERT(context != nullptr);
    Parser parser;
    parser_initialize(&parser, context);
    parser_set_view(&parser, source.ptr, source.length);
}
