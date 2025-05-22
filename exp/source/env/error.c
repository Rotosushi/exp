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
#include "env/error.h"
#include "support/ansi_colors.h"
#include "support/message.h"
#include "support/unreachable.h"

StringView error_code_to_view(ErrorCode code) {
    switch (code) {
    case ERROR_NONE: return SV("none");

    case ERROR_LEXER_ERROR_UNEXPECTED_CHAR:
        return SV("Unexcepted char in stream: ");
    case ERROR_LEXER_ERROR_UNMATCHED_DOUBLE_QUOTE:
        return SV("missing '\"' to end string literal.");

    case ERROR_PARSER_INTEGER_LITERAL_OUT_OF_RANGE:
        return SV("Integer literal exceeds capacity of i64.");

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
    case ERROR_PARSER_EXPECTED_TYPE: return SV("Expected a Type. Found: ");
    case ERROR_PARSER_EXPECTED_TOP_LEVEL_DECLARATION:
        return SV("Expected a Symbol Declaration. Found: ");
    case ERROR_PARSER_UNEXPECTED_TOKEN: return SV("Unexpected Token: ");

    case ERROR_ANALYSIS_UNINITIALIZED_VALUE:
        return SV("Cannot infer the type of an uninitialized Value.");
    case ERROR_ANALYSIS_UNDEFINED_SYMBOL:    return SV("Symbol Undefined: ");
    case ERROR_ANALYSIS_UNSUPPORTED_OPERAND: return SV("Operand Unsupported: ");
    case ERROR_ANALYSIS_OPERAND_IS_NOT_AN_INDEX:
        return SV("Operand is not an index: ");
    case ERROR_ANALYSIS_INDEX_OUT_OF_BOUNDS: return SV("Index out of bounds: ");
    case ERROR_ANALYSIS_TYPE_MISMATCH:
        return SV("Expected Type does not match Actual Type: ");

    case ERROR_EVALUATION_UNSIGNED_OVERFLOW:
        return SV("Unsigned Overflow/Underflow: ");
    case ERROR_EVALUATION_SIGNED_OVERFLOW:
        return SV("Signed Overflow/Underflow: ");

    default: EXP_UNREACHABLE();
    }
}

Error error_create() {
    Error error;
    error.code    = ERROR_NONE;
    error.message = string_create();
    return error;
}

Error error_construct(ErrorCode code, StringView sv) {
    Error error = error_create();
    error.code  = code;
    string_assign(&error.message, sv);
    return error;
}

Error error_from_string(ErrorCode code, String str) {
    Error error = {.code = code, .message = str};
    return error;
}

void error_destroy(Error *restrict error) {
    error->code = ERROR_NONE;
    string_destroy(&error->message);
}

void error_assign(Error *restrict error, ErrorCode code, StringView sv) {
    error->code = code;
    string_assign(&error->message, sv);
}

void error_assign_string(Error *restrict error, ErrorCode code, String str) {
    error->code    = code;
    error->message = str;
}

void error_print(Error const *restrict error, StringView file, u64 line) {
    String msg = string_create();
    string_append(&msg, SV(ANSI_COLOR_RED));
    string_append(&msg, error_code_to_view(error->code));
    string_append(&msg, SV(ANSI_COLOR_RESET));
    string_append(&msg, SV("["));
    string_append(&msg, SV(ANSI_COLOR_RED));
    string_append_string(&msg, &error->message);
    string_append(&msg, SV(ANSI_COLOR_RESET));
    string_append(&msg, SV("]"));
    message(MESSAGE_ERROR, file.ptr, line, string_to_view(&msg), stderr);
    string_destroy(&msg);
}
