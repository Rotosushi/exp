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
#include <ctype.h>
#include <string.h>

#include "frontend/lexer.h"

void lexer_init(Lexer *lexer) {
  lexer->cursor = lexer->token = NULL;
  lexer->line = 1;
}

void lexer_reset(Lexer *lexer) { lexer_init(lexer); }

void lexer_set_view(Lexer *lexer, const char *buffer) {
  lexer->cursor = lexer->token = buffer;
}

bool lexer_at_end(Lexer *lexer) { return *lexer->cursor == '\0'; }

size_t lexer_current_text_length(Lexer *lexer) {
  return (size_t)(lexer->cursor - lexer->token);
}

string_view lexer_current_text(Lexer *lexer) {
  string_view result = {lexer->token, lexer_current_text_length(lexer)};
  return result;
}

size_t lexer_current_line(Lexer *lexer) { return lexer->line; }

static bool isid(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static char lexer_next(Lexer *lexer) {
  lexer->cursor++;
  return lexer->cursor[-1];
}

static char lexer_peek(Lexer *lexer) { return *lexer->cursor; }

static char lexer_peek_next(Lexer *lexer) {
  if (lexer_at_end(lexer)) {
    return '\0';
  }

  return lexer->cursor[1];
}

static void lexer_skip_whitespace(Lexer *lexer) {
  while (1) {
    switch (lexer_peek(lexer)) {
    case '\n':
      lexer->line++;
      [[fallthrough]];
    case ' ':
    case '\r':
    case '\t':
      lexer_next(lexer);
      break;

    case '/':
      if (lexer_peek_next(lexer) == '/') {
        while ((lexer_peek(lexer) != '\n') && !lexer_at_end(lexer)) {
          lexer_next(lexer);
        }
        break;
      }
    default:
      return;
    }
  }
}

static bool lexer_match(Lexer *lexer, char c) {
  if (lexer_at_end(lexer)) {
    return 0;
  }

  if (lexer_peek(lexer) != c) {
    return 0;
  }

  lexer->cursor++;
  return 1;
}

static Token lexer_integer(Lexer *lexer) {
  while (isdigit(lexer_peek(lexer))) {
    lexer_next(lexer);
  }

  return TOK_INTEGER;
}

static Token lexer_check_keyword(Lexer *lexer, size_t begin, size_t length,
                                 const char *rest, Token keyword) {
  if ((lexer_current_text_length(lexer) == (begin + length)) &&
      (memcmp(lexer->token + begin, rest, length) == 0)) {
    return keyword;
  }

  return TOK_IDENTIFIER;
}

static Token lexer_identifier_or_keyword(Lexer *lexer) {
  switch (lexer->token[0]) {
  case 'f':
    if (lexer_current_text_length(lexer) > 1) {
      switch (lexer->token[1]) {
      case 'a':
        return lexer_check_keyword(lexer, 2, 3, "lse", TOK_FALSE);
      case 'n':
        return lexer_check_keyword(lexer, 2, 0, "", TOK_FN);
      }
    }
    break;

  case 'n':
    return lexer_check_keyword(lexer, 1, 2, "il", TOK_NIL);
  case 'r':
    return lexer_check_keyword(lexer, 1, 5, "eturn", TOK_RETURN);
  case 't':
    return lexer_check_keyword(lexer, 1, 3, "rue", TOK_TRUE);
  case 'v':
    return lexer_check_keyword(lexer, 1, 2, "ar", TOK_VAR);

  case 'B':
    return lexer_check_keyword(lexer, 1, 3, "ool", TOK_BOOL_TYPE);
  case 'I':
    return lexer_check_keyword(lexer, 1, 2, "nt", TOK_INT_TYPE);
  case 'N':
    return lexer_check_keyword(lexer, 1, 2, "il", TOK_NIL_TYPE);
  default:
    break;
  }

  return TOK_IDENTIFIER;
}

static Token lexer_identifier(Lexer *lexer) {
  while (isid(lexer_peek(lexer)) || isdigit(lexer_peek(lexer))) {
    lexer_next(lexer);
  }

  return lexer_identifier_or_keyword(lexer);
}

Token lexer_scan(Lexer *lexer) {
  lexer_skip_whitespace(lexer);
  lexer->token = lexer->cursor;

  if (lexer_at_end(lexer)) {
    return TOK_END;
  }

  char c = lexer_next(lexer);
  switch (c) {
  case '(':
    return TOK_BEGIN_PAREN;
  case ')':
    return TOK_END_PAREN;
  case '{':
    return TOK_BEGIN_BRACE;
  case '}':
    return TOK_END_BRACE;
  case ';':
    return TOK_SEMICOLON;
  case ':':
    return TOK_COLON;
  case ',':
    return TOK_COMMA;

  case '-':
    return lexer_match(lexer, '>') ? TOK_RIGHT_ARROW : TOK_MINUS;
  case '+':
    return TOK_PLUS;
  case '/':
    return TOK_SLASH;
  case '*':
    return TOK_STAR;

  case '!':
    return lexer_match(lexer, '=') ? TOK_BANG_EQUAL : TOK_BANG;
  case '=':
    return lexer_match(lexer, '=') ? TOK_EQUAL_EQUAL : TOK_EQUAL;
  case '<':
    return lexer_match(lexer, '=') ? TOK_LESS_EQUAL : TOK_LESS;
  case '>':
    return lexer_match(lexer, '=') ? TOK_GREATER_EQUAL : TOK_GREATER;
  case '&':
    return TOK_AND;
  case '|':
    return TOK_OR;
  case '^':
    return TOK_XOR;

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return lexer_integer(lexer);

  case '_':
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
    return lexer_identifier(lexer);

  default:
    break;
  }

  return TOK_ERROR;
}