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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_IMR_CALL_STACK_H
#define EXP_IMR_CALL_STACK_H
#include "env/locals.h"
#include "imr/function_body.h"

typedef struct CallFrame {
  FunctionBody *function;
  Frame frame;
} CallFrame;

typedef struct CallStack {
  u64 capacity;
  CallFrame *top;
  CallFrame *stack;
} CallStack;

CallStack call_stack_create();
void call_stack_destroy(CallStack *restrict cs);

bool call_stack_empty(CallStack *restrict cs);
CallFrame call_stack_push(CallStack *restrict cs, FunctionBody *fn,
                          Frame frame);
CallFrame call_stack_pop(CallStack *restrict cs);
CallFrame call_stack_top(CallStack *restrict cs);

#endif // !EXP_IMR_CALL_STACK_H