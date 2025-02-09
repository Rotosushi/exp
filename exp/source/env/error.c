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

#include "env/error.h"
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
    case ERROR_TYPECHECK_TUPLE_INDEX_NOT_IMMEDIATE:
        return SV("Tuple index is not known at compile time ");
    case ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS:
        return SV("Tuple index out of bounds ");
    case ERROR_TYPECHECK_RETURN_TYPE_UNKNOWN:
        return SV("functions return type could not be inferred");

    default: EXP_UNREACHABLE();
    }
}

void error_initialize(Error *error) {
    assert(error != nullptr);
    error->code = ERROR_NONE;
    string_initialize(&error->message);
}

void error_construct(Error *error, ErrorCode code, StringView sv) {
    assert(error != nullptr);
    error->code = code;
    string_from_view(&error->message, sv);
}

void error_from_string(Error *error, ErrorCode code, String str) {
    assert(error != nullptr);
    error->code    = code;
    error->message = str;
}

void error_terminate(Error *error) {
    error->code = ERROR_NONE;
    string_destroy(&error->message);
}

void error_assign(Error *error, ErrorCode code, StringView sv) {
    error->code = code;
    string_assign(&error->message, sv);
}

void error_print(Error *error, StringView file, u64 line) {
    String msg;
    string_initialize(&msg);
    string_append(&msg, error_code_sv(error->code));
    string_append(&msg, SV("["));
    string_append_string(&msg, &error->message);
    string_append(&msg, SV("]"));
    log_message(LOG_ERROR, file.ptr, line, string_to_cstring(&msg), stderr);
    string_destroy(&msg);
}
