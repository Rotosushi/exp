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

#include "codegen/x86/intrinsics/call.h"

void x86_codegen_call(Function const *restrict callee,
                      x86_Tuple const *restrict args,
                      u64 block_index,
                      x86_Function *restrict caller,
                      Context *restrict context) {
    // outgoing arguments are placed in registers if they
    // are small enough, and there are registers available.
    // otherwise they are pushed in reverse order onto the
    // stack. That is the first argument ends up farthest
    // down the stack.
}
