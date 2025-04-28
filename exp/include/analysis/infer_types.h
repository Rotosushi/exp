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
#ifndef EXP_ANALYSIS_INFER_TYPES_H
#define EXP_ANALYSIS_INFER_TYPES_H

#include "env/context.h"

/*
 * This applies to both infer_types and infer_lifetimes.
 * I want to add in an `evaluate(expr, context);` function
 * which is able to evaluate bytecode and return a result.
 * I think that as a part of that, infer_types and infer_lifetimes
 * need to have the API broken up into infer_types(expr, context);
 * and infer_lifetimes(expr, context);
 * But, what is an expression? I think, it can be an implicit lambda,
 * which captures the context. The thing is we need to rearchitect
 * the way that functions are defined. Namely function definitions
 * must be expressable within the bytecode itself. As const variables
 * already are. Then, we can pass a chunk of bytecode to the eval statement
 * as an expression, and that expression can be evaluated, one of the side
 * effects can then be, define a function, or define a global constant,
 * So what does that look like in practice though?
 * What is the memory layout of an operand that defines a function?
 * IF a Value can store a lambda, Then it can simply be a Constant in the
 * constants array. and we can have a unified 'const' instruction.
 *
 * approximate series of steps:
 * - rename 'const' instructions to 'let' instructions. (This aligns with the
 *   goal of making the language more 'functional')
 * - modify Value to store a Function.
 * - refactor the parser and the context, we need to generate
 *   a single chunk of bytecode for each "unit" of input, call
 *   that an 'expression'. we then evaluate each expression as
 *   it comes 'within' the context, this evaluation is what defines
 *   global names in the context, not the parser directly, as is done
 *   currently.
 */

/**
 * @brief Iterates through each defined function in the given
 * context, and fills in the type information for the function
 * and each SSA local.
 */
i32 infer_types(Context *restrict context);

#endif // !EXP_ANALYSIS_INFER_TYPES_H
