// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_FRONTEND_PARSER_H
#define EXP_FRONTEND_PARSER_H

#include "env/context.h"
#include "utility/result.h"

/*
  definition = "fn" identifier formal-args "->" type body

  formal-args = "(" formal-arg-list? ")"

  formal-arg-list = formal-arg ("," formal-arg)*

  formal-arg = identifier ":" type

  body = "{" statement* "}"

  statement = "return" expression ";"
            | "const" identifier "=" expression ";"
            | expression ";"

  expression = basic (binop precedece-parser)*

  basic = literal
        | unop basic
        | "(" expression ("," expression)* ")"
        | identifier

  literal = integer
          | "true"
          | "false"
          | "()"
*/

ExpResult parse_expression(StringView source, Context *context);

#endif // !EXP_FRONTEND_PARSER_H
