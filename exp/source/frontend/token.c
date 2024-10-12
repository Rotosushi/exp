

#include "frontend/token.h"
#include "utility/unreachable.h"

StringView token_to_view(Token token) {
  switch (token) {
  case TOK_END:            return SV("EOF");
  case TOK_BEGIN_COMMENT:  return SV("/*");
  case TOK_END_COMMENT:    return SV("*/");
  case TOK_BEGIN_PAREN:    return SV("(");
  case TOK_END_PAREN:      return SV(")");
  case TOK_BEGIN_BRACE:    return SV("{");
  case TOK_END_BRACE:      return SV("}");
  case TOK_DOT:            return SV(".");
  case TOK_COMMA:          return SV(",");
  case TOK_SEMICOLON:      return SV(";");
  case TOK_COLON:          return SV(":");
  case TOK_RIGHT_ARROW:    return SV("->");
  case TOK_MINUS:          return SV("-");
  case TOK_PLUS:           return SV("+");
  case TOK_SLASH:          return SV("/");
  case TOK_STAR:           return SV("*");
  case TOK_PERCENT:        return SV("%");
  case TOK_BANG:           return SV("!");
  case TOK_BANG_EQUAL:     return SV("!=");
  case TOK_EQUAL:          return SV("=");
  case TOK_EQUAL_EQUAL:    return SV("==");
  case TOK_GREATER:        return SV(">");
  case TOK_GREATER_EQUAL:  return SV(">=");
  case TOK_LESS:           return SV("<");
  case TOK_LESS_EQUAL:     return SV("<=");
  case TOK_AND:            return SV("&");
  case TOK_OR:             return SV("|");
  case TOK_XOR:            return SV("^");
  case TOK_FN:             return SV("fn");
  case TOK_VAR:            return SV("var");
  case TOK_CONST:          return SV("const");
  case TOK_RETURN:         return SV("return");
  case TOK_NIL:            return SV("()");
  case TOK_TRUE:           return SV("true");
  case TOK_FALSE:          return SV("false");
  case TOK_INTEGER:        return SV("<integral-literal>");
  case TOK_STRING_LITERAL: return SV("<string-literal>");
  case TOK_IDENTIFIER:     return SV("<identifier>");
  case TOK_TYPE_NIL:       return SV("Nil");
  case TOK_TYPE_BOOL:      return SV("Bool");
  case TOK_TYPE_I64:       return SV("i64");

  default: EXP_UNREACHABLE;
  }
}
