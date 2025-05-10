/**
 * Copyright (C) 2025 Cade Weinberg
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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "evaluate/evaluate.h"
#include "analysis/infer_lifetimes.h"
#include "analysis/infer_types.h"
#include "evaluate/top.h"
#include "support/assert.h"

/*
 * NOTE: We want to evaluate the given expression.
 * For a given binary operation this makes sense, because we
 * are operating on local variables. For instance
 *  `let x = 1 + 2;`
 * is compiled into
 * ```
 *     add %0, 1, 2
 *     let %x, %0
 * ```
 * The flow of data is through the local variables.
 * What about a function call operation? How does a value get
 * returned then? I think we need a stack. (Or maybe we could
 * use registers? But i'm unclear on how to do that without introducing
 * some of the complexity of register allocation to the evaluator.)
 * For now, lets use a stack. Thus, a call operation will retrieve
 * the result of the call via a pop. and the function will return a value
 * via a push.
 *
 * What about the top level expression? That is not really a function, in that
 * it does not have a return expression where we can write the push.
 * instead we expect the side effect of defining a new global symbol to happen.
 * so is it guaranteed that either `let` or `fn` are the last instruction
 * in a top level expression which defines a global symbol? Yes, based on
 * the way the parser is written, that must be the case. We currently do not
 * parse bare expressions at the top level. And we know that if the current
 * function has no name, it is a top level expression, and not a function
 * definition. So, if both are true, we could push a NIL onto the stack
 * and have that be the result of evaluation. But, This is only useful in
 * the context of a REPL, where we want to display the result of evaluation.
 * in this context we are only ever compiling the source code to a Target
 * assembly. Okay, it is also useful if we want to perform a ?strength
 * reduction? (is that what it's called?) Where we replace a call to a pure
 * function with comptime arguments with the result of it's evaluation. However,
 * in that case we would be calling a function, and thus there would be a return
 * statement, and thus we could pop the result off the stack.
 */

bool evaluate(Function *restrict expression, Context *restrict context) {
    exp_assert(expression != NULL);
    exp_assert(context != NULL);

    if (!infer_types(expression, context)) { return false; }
    if (!infer_lifetimes(expression)) { return false; }

    Frame frame;
    frame.function = expression;
    frame.index    = 0;
    frame.offset   = context_stack_size(context);
    frame.size     = 0;
    context_frames_push(context, frame);

    return evaluate_top_frame(context);
}
