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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "codegen/x86/imr/function.h"
#include "support/allocation.h"
#include "support/assert.h"

static void
x86_formal_argument_list_create(x86_FormalArgumentList *restrict args) {
    args->length = 0;
    args->buffer = NULL;
}

static void
x86_formal_arguments_destroy(x86_FormalArgumentList *restrict args) {
    deallocate(args->buffer);
    x86_formal_argument_list_create(args);
}

static void
x86_formal_argument_list_allocate(x86_FormalArgumentList *restrict args,
                                  u8 length) {
    args->length = length;
    args->buffer = callocate(length, sizeof(*args->buffer));
}

static x86_FormalArgument const *
x86_formal_argument_list_at(x86_FormalArgumentList const *restrict args,
                            u8 index) {
    exp_assert(args->length > index);
    return args->buffer + index;
}

void x86_function_create(x86_Function *restrict x86_function) {
    exp_assert(x86_function != NULL);
    x86_formal_argument_list_create(&x86_function->arguments);
    x86_bytecode_create(&x86_function->body);
    x86_gprp_create(&x86_function->gprp);
    x86_locations_create(&x86_function->locations);
}

void x86_function_destroy(x86_Function *restrict function) {
    exp_assert(function != NULL);
    x86_formal_arguments_destroy(&function->arguments);
    x86_bytecode_destroy(&function->body);
    x86_gprp_create(&function->gprp);
    x86_locations_destroy(&function->locations);
}

x86_FormalArgument const *
x86_function_formal_argument_at(x86_Function *restrict x86_function, u8 index) {
    exp_assert(x86_function != NULL);
    return x86_formal_argument_list_at(&x86_function->arguments, index);
}

void x86_function_append(x86_Function *restrict x86_function,
                         x86_Instruction instruction) {
    exp_assert(x86_function != NULL);
    x86_bytecode_append(&x86_function->body, instruction);
}

void x86_function_setup(x86_Function *restrict x86_function,
                        Function const *restrict function) {
    exp_assert(x86_function != NULL);
    exp_assert(function != NULL);
    // #NOTE: The argument list is the same length, unless we are passing in
    // a pointer to the stack space allocated for the return value as a hidden
    // first parameter.
    x86_formal_argument_list_allocate(&x86_function->arguments,
                                      function_arguments_length(function));
    x86_locations_allocate(&x86_function->locations,
                           function_locals_length(function));

    // #NOTE: Mark rsp and rbp as occupied, as these are used by the function
    // to implement it's stack frame at runtime.
    // #NOTE: #OPTIMIZATION: when the function uses no stack space, we can use
    // these registers as general purpose. So long as we save/restore them
    // before returning to another functions frame.
    // #NOTE: #OPTIMIZATION: if the frame size is static then, we can
    // get away with only using the RBP register. Which frees up the RSP for
    // general usage.
    exp_assert_always(x86_gprp_aquire(&x86_function->gprp, X86_GPR_RSP));
    exp_assert_always(x86_gprp_aquire(&x86_function->gprp, X86_GPR_RBP));
}

void x86_function_header(x86_Function *restrict x86_function) {
    // #NOTE: The standard x86 function header is to save the previous
    // stack frame, and save all registers used by the current
    // functions frame. Then allocate the current stack frame.
    // #ADDENDUM #[08-05-2025]:
    // Currently, we store all local values on the stack So the only values
    // appearing in registers are temporaries, and so are always retrieved
    // upon use, and thus never need to be save/restored. So all we have to do
    // here is save/restore the previous stack frames stack pointer. This will
    // need to change if we ever add an optimization which allows a local
    // variable to live in a register for it's lifetime.
    x86_function_append(x86_function,
                        x86_push(x86_operand_location_gpr(X86_GPR_RBP)));
    x86_function_append(x86_function,
                        x86_mov(x86_operand_location_gpr(X86_GPR_RBP),
                                x86_operand_location_gpr(X86_GPR_RSP)));
    // #NOTE: At this point in the function, we do not know how large the
    // functions frame is going to be. So we subtract a dummy value, to
    // be filled in after we perform all allocations on the stack,
    // though because our allocation strategy is so simple we might be able to
    // precompute the size, as we allocate all locals onto the stack.
    x86_function_append(
        x86_function,
        x86_sub(x86_operand_location_gpr(X86_GPR_RSP), x86_operand_i32(0)));
}

void x86_function_footer(x86_Function *restrict x86_function) {
    // #NOTE: since we save the previous rbp on the stack, then move the
    // rsp into rbp before subtracting the stack space for the function,
    // when we move rbp back into rsp we are in effect deallocating the
    // stack space we subtracted for the current frame. when we then pop
    // rbp back off the stack, this restores the base of the previous functions
    // call frame.
    // #NOTE: The standard x86 function footer is to restore the previous
    // stack and base pointers before returning to the previous functions frame.
    // A more optimal way is to use only rbp or only rsp given that we have
    // static size stack frames.
    x86_function_append(x86_function,
                        x86_mov(x86_operand_location_gpr(X86_GPR_RSP),
                                x86_operand_location_gpr(X86_GPR_RBP)));
    x86_function_append(x86_function,
                        x86_pop(x86_operand_location_gpr(X86_GPR_RBP)));
}
