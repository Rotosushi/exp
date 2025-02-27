// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_FRONTEND_TOKEN_H
#define EXP_FRONTEND_TOKEN_H

/**
 * @brief the Tokens that the Lexer returns
 */
typedef enum Token {
    TOK_END,

    TOK_ERROR_UNEXPECTED_CHAR,
    TOK_ERROR_UNMATCHED_DOUBLE_QUOTE,

    TOK_BEGIN_COMMENT,
    TOK_END_COMMENT,
    TOK_BEGIN_PAREN,
    TOK_END_PAREN,
    TOK_BEGIN_BRACE,
    TOK_END_BRACE,
    TOK_DOT,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_COLON,
    TOK_RIGHT_ARROW,

    TOK_MINUS,
    TOK_PLUS,
    TOK_SLASH,
    TOK_STAR,
    TOK_PERCENT,
    TOK_BANG,
    TOK_BANG_EQUAL,
    TOK_EQUAL,
    TOK_EQUAL_EQUAL,
    TOK_GREATER,
    TOK_GREATER_EQUAL,
    TOK_LESS,
    TOK_LESS_EQUAL,
    TOK_AND,
    TOK_OR,
    TOK_XOR,

    TOK_FN,
    TOK_VAR,
    TOK_CONST,
    TOK_RETURN,

    TOK_NIL,
    TOK_TRUE,
    TOK_FALSE,
    TOK_INTEGER,
    TOK_STRING_LITERAL,
    TOK_IDENTIFIER,

    TOK_TYPE_NIL,
    TOK_TYPE_BOOL,
    TOK_TYPE_I32,
} Token;

#endif // !EXP_FRONTEND_TOKEN_H
