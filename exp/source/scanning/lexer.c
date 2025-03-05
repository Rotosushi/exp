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
#include <ctype.h>
#include <string.h>

#include "scanning/lexer.h"

Lexer lexer_create() {
    Lexer lexer;
    lexer_init(&lexer);
    return lexer;
}

void lexer_init(Lexer *restrict lexer) {
    assert(lexer != NULL);
    lexer->length = 0;
    lexer->buffer = lexer->cursor = lexer->token = NULL;
    lexer->line = lexer->column = 1;
}

void lexer_reset(Lexer *restrict lexer) {
    assert(lexer != NULL);
    lexer_init(lexer);
}

void lexer_set_view(Lexer *restrict lexer, char const *buffer, u64 length) {
    assert(lexer != NULL);
    lexer->buffer = lexer->cursor = lexer->token = buffer;
    lexer->length                                = length;
}

bool lexer_at_end(Lexer *restrict lexer) {
    assert(lexer != NULL);
    return (u64)(lexer->cursor - lexer->buffer) >= lexer->length;
}

static u64 lexer_current_text_length(Lexer const *restrict lexer) {
    return (u64)(lexer->cursor - lexer->token);
}

StringView lexer_current_text(Lexer const *restrict lexer) {
    assert(lexer != NULL);
    StringView result =
        string_view_from_str(lexer->token, lexer_current_text_length(lexer));
    return result;
}

u64 lexer_current_line(Lexer const *restrict lexer) {
    assert(lexer != NULL);
    return lexer->line;
}

u64 lexer_current_column(Lexer const *restrict lexer) {
    assert(lexer != NULL);
    return lexer->column;
}

static bool isid(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static char lexer_next(Lexer *restrict lexer) {
    lexer->column++;
    lexer->cursor++;
    return lexer->cursor[-1];
}

static char lexer_peek(Lexer *restrict lexer) { return *lexer->cursor; }

static char lexer_peek_next(Lexer *restrict lexer) {
    if (lexer_at_end(lexer)) { return '\0'; }

    return lexer->cursor[1];
}

static void lexer_skip_whitespace(Lexer *restrict lexer) {
    while (1) {
        if (lexer_at_end(lexer)) break;
        switch (lexer_peek(lexer)) {
        case '\n':
            lexer->column = 1;
            lexer->line++;
            [[fallthrough]];
        case ' ':
        case '\r':
        case '\t': lexer_next(lexer); break;

        case '/':
            if (lexer_peek_next(lexer) == '/') {
                while ((lexer_peek(lexer) != '\n') && !lexer_at_end(lexer)) {
                    lexer_next(lexer);
                }
                break;
            }
        default: return;
        }
    }
}

static bool lexer_match(Lexer *restrict lexer, char c) {
    if (lexer_at_end(lexer)) { return 0; }

    if (lexer_peek(lexer) != c) { return 0; }

    lexer->column++;
    lexer->cursor++;
    return 1;
}

static Token lexer_integer(Lexer *restrict lexer) {
    while (isdigit(lexer_peek(lexer))) {
        lexer_next(lexer);
    }

    return TOK_INTEGER;
}

static Token lexer_check_keyword(Lexer *restrict lexer,
                                 u64 begin,
                                 u64 length,
                                 const char *rest,
                                 Token keyword) {
    if ((lexer_current_text_length(lexer) == (begin + length)) &&
        (memcmp(lexer->token + begin, rest, length) == 0)) {
        return keyword;
    }

    return TOK_IDENTIFIER;
}

static Token lexer_identifier_or_keyword(Lexer *restrict lexer) {
    switch (lexer->token[0]) {
    case 'b': return lexer_check_keyword(lexer, 1, 3, "ool", TOK_TYPE_BOOL);
    case 'c': return lexer_check_keyword(lexer, 1, 4, "onst", TOK_CONST);
    case 'f':
        if (lexer_current_text_length(lexer) > 1) {
            switch (lexer->token[1]) {
            case 'a': return lexer_check_keyword(lexer, 2, 3, "lse", TOK_FALSE);
            case 'n': return lexer_check_keyword(lexer, 2, 0, "", TOK_FN);
            default:  break;
            }
        }
        break;

    case 'i': return lexer_check_keyword(lexer, 1, 2, "64", TOK_TYPE_I64);
    case 'n': return lexer_check_keyword(lexer, 1, 2, "il", TOK_TYPE_NIL);
    case 'r': return lexer_check_keyword(lexer, 1, 5, "eturn", TOK_RETURN);
    case 't': return lexer_check_keyword(lexer, 1, 3, "rue", TOK_TRUE);
    case 'v': return lexer_check_keyword(lexer, 1, 2, "ar", TOK_VAR);

    default: break;
    }

    return TOK_IDENTIFIER;
}

static Token lexer_string_literal(Lexer *restrict lexer) {
    // eat the '"'
    lexer->token++;
    // #TODO handle escape sequences
    while (lexer_peek(lexer) != '"') {
        lexer_next(lexer);

        // unmatched '"' in token stream.
        if (lexer_at_end(lexer)) { return TOK_ERROR_UNMATCHED_DOUBLE_QUOTE; }
    }
    // eat the '"'
    lexer->column++;
    lexer->cursor++;

    return TOK_STRING_LITERAL;
}

static Token lexer_identifier(Lexer *restrict lexer) {
    while (isid(lexer_peek(lexer)) || isdigit(lexer_peek(lexer))) {
        lexer_next(lexer);
    }

    return lexer_identifier_or_keyword(lexer);
}

Token lexer_scan(Lexer *restrict lexer) {
    assert(lexer != NULL);
    if (lexer_at_end(lexer)) { return TOK_END; }

    lexer_skip_whitespace(lexer);
    lexer->token = lexer->cursor;

    if (lexer_at_end(lexer)) { return TOK_END; }

    char c = lexer_next(lexer);
    switch (c) {
    case '(': return lexer_match(lexer, ')') ? TOK_NIL : TOK_BEGIN_PAREN;
    case ')': return TOK_END_PAREN;
    case '{': return TOK_BEGIN_BRACE;
    case '}': return TOK_END_BRACE;
    case ';': return TOK_SEMICOLON;
    case ':': return TOK_COLON;
    case ',': return TOK_COMMA;
    case '.': return TOK_DOT;

    case '-': return lexer_match(lexer, '>') ? TOK_RIGHT_ARROW : TOK_MINUS;
    case '+': return TOK_PLUS;
    case '/': return lexer_match(lexer, '*') ? TOK_BEGIN_COMMENT : TOK_SLASH;
    case '*': return lexer_match(lexer, '/') ? TOK_END_COMMENT : TOK_STAR;
    case '%': return TOK_PERCENT;

    case '!': return lexer_match(lexer, '=') ? TOK_BANG_EQUAL : TOK_BANG;
    case '=': return lexer_match(lexer, '=') ? TOK_EQUAL_EQUAL : TOK_EQUAL;
    case '<': return lexer_match(lexer, '=') ? TOK_LESS_EQUAL : TOK_LESS;
    case '>': return lexer_match(lexer, '=') ? TOK_GREATER_EQUAL : TOK_GREATER;
    case '&': return TOK_AND;
    case '|': return TOK_OR;
    case '^': return TOK_XOR;

    case '"': return lexer_string_literal(lexer);

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': return lexer_integer(lexer);

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
    case 'Z': return lexer_identifier(lexer);

    default: return TOK_ERROR_UNEXPECTED_CHAR;
    }
}
