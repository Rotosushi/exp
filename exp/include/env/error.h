// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_ERROR_H
#define EXP_ENV_ERROR_H

#include "support/string.h"

typedef enum ErrorCode : u8 {
    ERROR_NONE,

    ERROR_LEXER_ERROR_UNEXPECTED_CHAR,
    ERROR_LEXER_ERROR_UNMATCHED_DOUBLE_QUOTE,

    ERROR_PARSER_INTEGER_LITERAL_OUT_OF_RANGE,

    ERROR_PARSER_EXPECTED_END_COMMENT,
    ERROR_PARSER_EXPECTED_BEGIN_BRACE,
    ERROR_PARSER_EXPECTED_END_BRACE,
    ERROR_PARSER_EXPECTED_BEGIN_PAREN,
    ERROR_PARSER_EXPECTED_END_PAREN,
    ERROR_PARSER_EXPECTED_RIGHT_ARROW,
    ERROR_PARSER_EXPECTED_SEMICOLON,
    ERROR_PARSER_EXPECTED_COLON,
    ERROR_PARSER_EXPECTED_EQUAL,

    ERROR_PARSER_EXPECTED_KEYWORD_CONST,
    ERROR_PARSER_EXPECTED_KEYWORD_FN,

    ERROR_PARSER_EXPECTED_EXPRESSION,
    ERROR_PARSER_EXPECTED_STATEMENT,
    ERROR_PARSER_EXPECTED_IDENTIFIER,
    ERROR_PARSER_EXPECTED_TYPE,
    ERROR_PARSER_EXPECTED_TOP_LEVEL_DECLARATION,

    ERROR_PARSER_UNEXPECTED_TOKEN,

    ERROR_ANALYSIS_UNDEFINED_SYMBOL,
    ERROR_ANALYSIS_UNSUPPORTED_OPERAND,
    ERROR_ANALYSIS_TYPE_MISMATCH,
    ERROR_ANALYSIS_OPERAND_IS_NOT_AN_INDEX,
    ERROR_ANALYSIS_INDEX_OUT_OF_BOUNDS,

    ERROR_EVALUATION_UNSIGNED_OVERFLOW,
    ERROR_EVALUATION_SIGNED_OVERFLOW,
} ErrorCode;

StringView error_code_to_view(ErrorCode code);

/**
 * @brief models errors which are produced during compilation.
 * designed to allow the creation of informative and readable
 * error messages to users of the compiler.
 *
 * @note this is not intended to model errors that are internal
 * to the compiler, and signal code defects within the compiler.
 * The general strategy for internal errors is to call panic.
 *
 */
typedef struct Error {
    ErrorCode code;
    String    message;
} Error;

Error error_create();
Error error_construct(ErrorCode code, StringView sv);
Error error_from_string(ErrorCode code, String str);
void  error_destroy(Error *restrict error);

void error_assign(Error *restrict error, ErrorCode code, StringView sv);
void error_assign_string(Error *restrict error, ErrorCode code, String str);

void error_print(Error const *restrict error, StringView file, u64 line);

#endif // !EXP_ENV_ERROR_H
