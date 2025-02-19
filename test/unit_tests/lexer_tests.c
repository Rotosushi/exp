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
#include <stdio.h>
#include <string.h>

#include "frontend/lexer.h"
#include "utility/result.h"

static i32 test_lexer_scans_token(const char *buffer, Token token) {
    Lexer lexer = lexer_create();
    lexer_set_view(&lexer, buffer, strlen(buffer));

    Token scanned = lexer_scan(&lexer);
    if (scanned != token) {
        fputs("failed match: ", stderr);
        fputs("\n", stderr);
        fputs(buffer, stderr);
        fputs("\n", stderr);
        return 1;
    } else {
        return 0;
    }
}

i32 lexer_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
    i32 result = 0;

    result += test_lexer_scans_token("(", TOK_BEGIN_PAREN);
    result += test_lexer_scans_token(")", TOK_END_PAREN);
    result += test_lexer_scans_token("{", TOK_BEGIN_BRACE);
    result += test_lexer_scans_token("}", TOK_END_BRACE);
    result += test_lexer_scans_token(",", TOK_COMMA);
    result += test_lexer_scans_token(";", TOK_SEMICOLON);
    result += test_lexer_scans_token(":", TOK_COLON);
    result += test_lexer_scans_token("!", TOK_BANG);
    result += test_lexer_scans_token("!=", TOK_BANG_EQUAL);
    result += test_lexer_scans_token("=", TOK_EQUAL);
    result += test_lexer_scans_token("==", TOK_EQUAL_EQUAL);
    result += test_lexer_scans_token("<", TOK_LESS);
    result += test_lexer_scans_token("<=", TOK_LESS_EQUAL);
    result += test_lexer_scans_token(">", TOK_GREATER);
    result += test_lexer_scans_token(">=", TOK_GREATER_EQUAL);
    result += test_lexer_scans_token("&", TOK_AND);
    result += test_lexer_scans_token("|", TOK_OR);
    result += test_lexer_scans_token("^", TOK_XOR);
    result += test_lexer_scans_token("-", TOK_MINUS);
    result += test_lexer_scans_token("->", TOK_RIGHT_ARROW);
    result += test_lexer_scans_token("+", TOK_PLUS);
    result += test_lexer_scans_token("/", TOK_SLASH);
    result += test_lexer_scans_token("*", TOK_STAR);

    result += test_lexer_scans_token("fn", TOK_FN);
    result += test_lexer_scans_token("f", TOK_IDENTIFIER);
    result += test_lexer_scans_token("fnt", TOK_IDENTIFIER);
    result += test_lexer_scans_token("f32", TOK_TYPE_F32);
    result += test_lexer_scans_token("f3", TOK_IDENTIFIER);
    result += test_lexer_scans_token("f32l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("f64", TOK_TYPE_F64);
    result += test_lexer_scans_token("f6", TOK_IDENTIFIER);
    result += test_lexer_scans_token("f64l", TOK_IDENTIFIER);

    result += test_lexer_scans_token("var", TOK_VAR);
    result += test_lexer_scans_token("va", TOK_IDENTIFIER);
    result += test_lexer_scans_token("varl", TOK_IDENTIFIER);
    result += test_lexer_scans_token("const", TOK_CONST);
    result += test_lexer_scans_token("con", TOK_IDENTIFIER);
    result += test_lexer_scans_token("constl", TOK_IDENTIFIER);
    result += test_lexer_scans_token("return", TOK_RETURN);
    result += test_lexer_scans_token("relurn", TOK_IDENTIFIER);
    result += test_lexer_scans_token("returnl", TOK_IDENTIFIER);
    result += test_lexer_scans_token("nil", TOK_TYPE_NIL);
    result += test_lexer_scans_token("ni", TOK_IDENTIFIER);
    result += test_lexer_scans_token("nill", TOK_IDENTIFIER);
    result += test_lexer_scans_token("true", TOK_TRUE);
    result += test_lexer_scans_token("tru", TOK_IDENTIFIER);
    result += test_lexer_scans_token("truel", TOK_IDENTIFIER);
    result += test_lexer_scans_token("false", TOK_FALSE);
    result += test_lexer_scans_token("fals", TOK_IDENTIFIER);
    result += test_lexer_scans_token("falsel", TOK_IDENTIFIER);
    result += test_lexer_scans_token("bool", TOK_TYPE_BOOL);
    result += test_lexer_scans_token("boo", TOK_IDENTIFIER);
    result += test_lexer_scans_token("booll", TOK_IDENTIFIER);

    result += test_lexer_scans_token("i", TOK_IDENTIFIER);
    result += test_lexer_scans_token("i8", TOK_TYPE_I8);
    result += test_lexer_scans_token("i8l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("i16", TOK_TYPE_I16);
    result += test_lexer_scans_token("i1", TOK_IDENTIFIER);
    result += test_lexer_scans_token("i16l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("i32", TOK_TYPE_I32);
    result += test_lexer_scans_token("i3", TOK_IDENTIFIER);
    result += test_lexer_scans_token("i32l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("i64", TOK_TYPE_I64);
    result += test_lexer_scans_token("i6", TOK_IDENTIFIER);
    result += test_lexer_scans_token("i64l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u8", TOK_TYPE_U8);
    result += test_lexer_scans_token("u8l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u16", TOK_TYPE_U16);
    result += test_lexer_scans_token("u1", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u16l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u32", TOK_TYPE_U32);
    result += test_lexer_scans_token("u3", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u32l", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u64", TOK_TYPE_U64);
    result += test_lexer_scans_token("u6", TOK_IDENTIFIER);
    result += test_lexer_scans_token("u64l", TOK_IDENTIFIER);

    result += test_lexer_scans_token("hello", TOK_IDENTIFIER);
    result += test_lexer_scans_token("hello1232", TOK_IDENTIFIER);
    result += test_lexer_scans_token("9473289", TOK_INTEGER);
    result += test_lexer_scans_token("\"hello world!\"", TOK_STRING_LITERAL);

    return result;
}
