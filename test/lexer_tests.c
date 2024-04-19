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

#include "frontend/lexer.h"

// return true on failure
static bool test_lexer_scans_token(const char *buffer, Token token) {
  Lexer lexer = lexer_create();
  lexer_set_view(&lexer, buffer);

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
  bool failed = 0;

  failed |= test_lexer_scans_token("(", TOK_BEGIN_PAREN);
  failed |= test_lexer_scans_token(")", TOK_END_PAREN);
  failed |= test_lexer_scans_token("{", TOK_BEGIN_BRACE);
  failed |= test_lexer_scans_token("}", TOK_END_BRACE);
  failed |= test_lexer_scans_token(",", TOK_COMMA);
  failed |= test_lexer_scans_token(";", TOK_SEMICOLON);
  failed |= test_lexer_scans_token(":", TOK_COLON);
  failed |= test_lexer_scans_token("!", TOK_BANG);
  failed |= test_lexer_scans_token("!=", TOK_BANG_EQUAL);
  failed |= test_lexer_scans_token("=", TOK_EQUAL);
  failed |= test_lexer_scans_token("==", TOK_EQUAL_EQUAL);
  failed |= test_lexer_scans_token("<", TOK_LESS);
  failed |= test_lexer_scans_token("<=", TOK_LESS_EQUAL);
  failed |= test_lexer_scans_token(">", TOK_GREATER);
  failed |= test_lexer_scans_token(">=", TOK_GREATER_EQUAL);
  failed |= test_lexer_scans_token("&", TOK_AND);
  failed |= test_lexer_scans_token("|", TOK_OR);
  failed |= test_lexer_scans_token("^", TOK_XOR);
  failed |= test_lexer_scans_token("-", TOK_MINUS);
  failed |= test_lexer_scans_token("->", TOK_RIGHT_ARROW);
  failed |= test_lexer_scans_token("+", TOK_PLUS);
  failed |= test_lexer_scans_token("/", TOK_SLASH);
  failed |= test_lexer_scans_token("*", TOK_STAR);

  failed |= test_lexer_scans_token("fn", TOK_FN);
  failed |= test_lexer_scans_token("f", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("fnt", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("var", TOK_VAR);
  failed |= test_lexer_scans_token("va", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("varl", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("return", TOK_RETURN);
  failed |= test_lexer_scans_token("relurn", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("returnl", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("nil", TOK_TYPE_NIL);
  failed |= test_lexer_scans_token("ni", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("nill", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("true", TOK_TRUE);
  failed |= test_lexer_scans_token("tru", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("truel", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("false", TOK_FALSE);
  failed |= test_lexer_scans_token("fals", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("falsel", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("bool", TOK_TYPE_BOOL);
  failed |= test_lexer_scans_token("boo", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("booll", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("i64", TOK_TYPE_I64);
  failed |= test_lexer_scans_token("i63", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("i", TOK_IDENTIFIER);

  failed |= test_lexer_scans_token("hello", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("hello1232", TOK_IDENTIFIER);
  failed |= test_lexer_scans_token("9473289", TOK_INTEGER);

  failed |= test_lexer_scans_token("\"hello world!\"", TOK_STRING_LITERAL);

  if (failed) {
    return 1;
  } else {
    return 0;
  }
}