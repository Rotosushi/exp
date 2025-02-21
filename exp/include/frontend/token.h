// Copyright (C) 2025 Cade Weinberg
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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file frontend/token.h
 */

#ifndef EXP_FRONTEND_TOKEN_H
#define EXP_FRONTEND_TOKEN_H

/**
 * @brief the Tokens that the Lexer returns
 */
typedef enum Token {
    TOK_END,

    TOK_ERROR_UNEXPECTED_CHAR,
    TOK_ERROR_UNMATCHED_DOUBLE_QUOTE,
    TOK_ERROR_DOUBLE_DOT_IN_REAL,
    TOK_ERROR_UNKNOWN_LITERAL_TYPE_SPECIFIER,

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
    TOK_TYPE_I8,
    TOK_TYPE_I16,
    TOK_TYPE_I32,
    TOK_TYPE_I64,
    TOK_TYPE_U8,
    TOK_TYPE_U16,
    TOK_TYPE_U32,
    TOK_TYPE_U64,
    TOK_TYPE_F32,
    TOK_TYPE_F64,
} Token;

#endif // !EXP_FRONTEND_TOKEN_H
