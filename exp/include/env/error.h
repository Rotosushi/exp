// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ENV_ERROR_H
#define EXP_ENV_ERROR_H

#include "utility/string.h"

typedef enum ErrorCode : u8 {
    ERROR_NONE,

    ERROR_LEXER_ERROR_UNEXPECTED_CHAR,
    ERROR_LEXER_ERROR_UNMATCHED_DOUBLE_QUOTE,

    ERROR_INTEGER_TO_LARGE,

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

    ERROR_PARSER_UNEXPECTED_TOKEN,

    ERROR_TYPECHECK_UNDEFINED_SYMBOL,
    ERROR_TYPECHECK_TYPE_MISMATCH,
    ERROR_TYPECHECK_TYPE_NOT_CALLABLE,
    ERROR_TYPECHECK_TYPE_NOT_INDEXABLE,
    ERROR_TYPECHECK_TUPLE_INDEX_NOT_IMMEDIATE,
    ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS,
    ERROR_TYPECHECK_RETURN_TYPE_UNKNOWN,
} ErrorCode;

StringView error_code_sv(ErrorCode code);

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
    String message;
} Error;

void error_initialize(Error *error);
void error_construct(Error *error, ErrorCode code, StringView sv);
void error_from_string(Error *error, ErrorCode code, String str);
void error_terminate(Error *error);

void error_assign(Error *error, ErrorCode code, StringView sv);

void print_error(String *buffer, Error *error, StringView file, u64 line);

#endif // !EXP_ENV_ERROR_H
