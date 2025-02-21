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
 * @file env/error.c
 */
#include "env/error.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

StringView error_code_sv(ErrorCode code) {
    switch (code) {
    case ERROR_NONE: return SV("none");

    case ERROR_LEXER_ERROR_UNEXPECTED_CHAR:
        return SV("Unexcepted char in stream: ");
    case ERROR_LEXER_ERROR_UNMATCHED_DOUBLE_QUOTE:
        return SV("missing '\"' to end string literal.");

    case ERROR_INTEGER_TO_LARGE: return SV("Integer literal too large.");

    case ERROR_PARSER_EXPECTED_END_COMMENT:
        return SV("Expected: [*/]. Found: ");
    case ERROR_PARSER_EXPECTED_BEGIN_BRACE: return SV("Expected: [{]. Found: ");
    case ERROR_PARSER_EXPECTED_END_BRACE:   return SV("Expected: [}]. Found: ");
    case ERROR_PARSER_EXPECTED_BEGIN_PAREN: return SV("Expected: [(]. Found: ");
    case ERROR_PARSER_EXPECTED_END_PAREN:   return SV("Expected: [)]. Found: ");
    case ERROR_PARSER_EXPECTED_RIGHT_ARROW:
        return SV("Expected: [->]. Found: ");
    case ERROR_PARSER_EXPECTED_SEMICOLON: return SV("Expected: [;]. Found: ");
    case ERROR_PARSER_EXPECTED_COLON:     return SV("Expected: [:]. Found: ");
    case ERROR_PARSER_EXPECTED_EQUAL:     return SV("Expected: [=]. Found: ");
    case ERROR_PARSER_EXPECTED_KEYWORD_CONST:
        return SV("Expected: [const]. Found: ");
    case ERROR_PARSER_EXPECTED_KEYWORD_FN: return SV("Expected: [Fn]. Found: ");
    case ERROR_PARSER_EXPECTED_EXPRESSION:
        return SV("Expected an Expression. Found: ");
    case ERROR_PARSER_EXPECTED_STATEMENT:
        return SV("Expected a Statement. Found: ");
    case ERROR_PARSER_EXPECTED_IDENTIFIER:
        return SV("Expected an Identifier. Found: ");
    case ERROR_PARSER_UNEXPECTED_TOKEN: return SV("Unexpected Token: ");

    case ERROR_TYPECHECK_UNDEFINED_SYMBOL: return SV("Symbol Undefined: ");
    case ERROR_TYPECHECK_TYPE_MISMATCH:
        return SV("Expected Type does not match Actual Type: ");
    case ERROR_TYPECHECK_TYPE_NOT_CALLABLE: return SV("Type is not Callable ");
    case ERROR_TYPECHECK_TYPE_NOT_INDEXABLE:
        return SV("Type is not Indexable ");
    case ERROR_TYPECHECK_DOT_ARGUMENT_NOT_AN_INDEX:
        return SV("Argument to dot operator is not an index ");
    case ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS:
        return SV("Tuple index out of bounds ");
    case ERROR_TYPECHECK_RETURN_TYPE_UNKNOWN:
        return SV("functions return type could not be inferred");
    case ERROR_TYPECHECK_TYPE_NOT_ARITHMETIC:
        return SV("Type is not Arithmetic ");
    case ERROR_TYPECHECK_TYPE_NOT_SIGNED: return SV("Type is not Signed ");

    default: EXP_UNREACHABLE();
    }
}

void error_initialize(Error *error) {
    EXP_ASSERT(error != nullptr);
    error->code = ERROR_NONE;
    string_initialize(&error->message);
}

void error_construct(Error *error, ErrorCode code, StringView sv) {
    EXP_ASSERT(error != nullptr);
    error->code = code;
    string_from_view(&error->message, sv);
}

void error_from_string(Error *error, ErrorCode code, String str) {
    EXP_ASSERT(error != nullptr);
    error->code    = code;
    error->message = str;
}

void error_terminate(Error *error) {
    EXP_ASSERT(error != nullptr);
    error->code = ERROR_NONE;
    string_terminate(&error->message);
}

void error_assign(Error *error, ErrorCode code, StringView sv) {
    EXP_ASSERT(error != nullptr);
    error->code = code;
    string_assign(&error->message, sv);
}

void print_error(String *buffer, Error *error, StringView file, u64 line) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(error != nullptr);
    string_append(buffer, SV("[error @"));
    string_append(buffer, file);
    string_append(buffer, SV(":"));
    string_append_u64(buffer, line);
    string_append(buffer, SV("] "));
    string_append(buffer, error_code_sv(error->code));
    string_append_string(buffer, &error->message);
}
