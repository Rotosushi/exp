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
#include "codegen/x86/imr/allocation.h"
#include "codegen/x86/imr/local_allocator.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

static void
x86_formal_argument_list_create(x86_FormalArgumentList *restrict args) {
    args->length   = 0;
    args->capacity = 0;
    args->buffer   = NULL;
}

static void
x86_formal_arguments_destroy(x86_FormalArgumentList *restrict args) {
    deallocate(args->buffer);
    x86_formal_argument_list_create(args);
}

static bool x86_formal_arguments_full(x86_FormalArgumentList *restrict args) {
    return (args->length + 1) >= args->capacity;
}

static void x86_formal_arguments_grow(x86_FormalArgumentList *restrict args) {
    Growth_u8 g    = array_growth_u8(args->capacity, sizeof(*args->buffer));
    args->buffer   = reallocate(args->buffer, g.alloc_size);
    args->capacity = g.new_capacity;
}

void x86_formal_argument_list_append(x86_FormalArgumentList *restrict args,
                                     x86_Allocation *arg) {
    exp_assert(args != NULL);
    exp_assert(arg != NULL);
    if (x86_formal_arguments_full(args)) { x86_formal_arguments_grow(args); }
    args->buffer[args->length++] = arg;
}

static x86_Allocation *
x86_formal_argument_list_at(x86_FormalArgumentList *restrict args, u8 index) {
    exp_assert(args->length > index);
    return args->buffer[index];
}

void x86_function_create(x86_Function *restrict function) {
    exp_assert(function != NULL);
    x86_formal_argument_list_create(&function->arguments);
    x86_body_create(&function->body);
    x86_local_allocator_create(&function->local_allocator);
    function->result = NULL;
}

void x86_function_destroy(x86_Function *restrict function) {
    exp_assert(function != NULL);
    x86_formal_arguments_destroy(&function->arguments);
    x86_body_destroy(&function->body);
    x86_local_allocator_create(&function->local_allocator);
    function->result = NULL;
}

x86_Allocation *
x86_function_formal_argument_at(x86_Function *restrict x86_function, u8 index) {
    exp_assert(x86_function != NULL);
    return x86_formal_argument_list_at(&x86_function->arguments, index);
}

x86_Allocation *x86_function_allocation_at(x86_Function *restrict x86_function,
                                           u32 ssa) {
    exp_assert(x86_function != NULL);
    return x86_local_allocator_allocation_at(&x86_function->local_allocator,
                                             ssa);
}

x86_Allocation *
x86_function_allocation_named(x86_Function *restrict x86_function,
                              ConstantString const *name) {
    exp_assert(x86_function != NULL);
    return x86_local_allocator_allocation_named(&x86_function->local_allocator,
                                                name);
}

void x86_function_insert_block(x86_Function *restrict x86_function,
                               u32 position) {
    exp_assert(x86_function != NULL);
    x86_body_insert(&x86_function->body, position);
}

void x86_function_prepend_block(x86_Function *restrict x86_function) {
    exp_assert(x86_function != NULL);
    x86_body_prepend(&x86_function->body);
}

u32 x86_function_append_block(x86_Function *restrict x86_function) {
    exp_assert(x86_function != NULL);
    return x86_body_append(&x86_function->body);
}

void x86_function_target_block(x86_Function *restrict x86_function, u32 block) {
    exp_assert(x86_function != NULL);
    exp_assert(block < x86_function->body.length);
    x86_function->current_block = block;
}

u32 x86_function_current_block(x86_Function *restrict x86_function) {
    exp_assert(x86_function != NULL);
    return x86_function->current_block;
}

void x86_function_insert(x86_Function *restrict function,
                         x86_Instruction instruction,
                         u32             block_index) {
    exp_assert(function != NULL);
    x86_Block *block = x86_body_at(&function->body, function->current_block);
    x86_block_insert(block, instruction, block_index);
}

void x86_function_prepend(x86_Function *restrict function,
                          x86_Instruction instruction) {
    exp_assert(function != NULL);
    x86_Block *block = x86_body_at(&function->body, function->current_block);
    x86_block_prepend(block, instruction);
}

void x86_function_append(x86_Function *restrict function,
                         x86_Instruction instruction) {
    exp_assert(function != NULL);
    x86_Block *block = x86_body_at(&function->body, function->current_block);
    x86_block_append(block, instruction);
}

void x86_function_setup(x86_Function *restrict x86_function,
                        Function const *restrict function) {
    exp_assert(x86_function != NULL);
    exp_assert(function != NULL);

    x86_function->name = function->name;

    // #NOTE: Mark rsp and rbp as occupied, as these are used by the function
    // to implement it's stack frame at runtime.
    // #NOTE: #OPTIMIZATION: when the function uses no stack space, we can use
    // these registers as general purpose. So long as we save/restore them
    // before returning to another functions frame.
    // #NOTE: #OPTIMIZATION: if the frame size is static then we can
    // get away with only using the RBP register. Which frees up the RSP for
    // general usage.
    x86_local_allocator_aquire_gpr(&x86_function->local_allocator, X86_GPR_RSP);
    x86_local_allocator_aquire_gpr(&x86_function->local_allocator, X86_GPR_RBP);

    x86_local_allocator_allocate_result(&x86_function->local_allocator,
                                        function->result,
                                        &x86_function->arguments);

    x86_local_allocator_allocate_incoming_arguments(
        &x86_function->local_allocator,
        &function->arguments,
        &x86_function->arguments);

    u32 block = x86_function_append_block(x86_function);
    x86_function_target_block(x86_function, block);
}

void x86_function_header(x86_Function *restrict function) {
    exp_assert(function != NULL);
    // Set up the function to insert instructions in the new first
    // basic block.
    u32 previous_block = function->current_block;
    x86_function_prepend_block(function);
    x86_function_target_block(function, 0);
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
    x86_function_append(function,
                        x86_push(x86_operand_location_gpr(X86_GPR_RBP)));
    x86_function_append(function,
                        x86_mov(x86_operand_location_gpr(X86_GPR_RBP),
                                x86_operand_location_gpr(X86_GPR_RSP)));

    i32 stack_size = x86_local_allocator_stack_size(&function->local_allocator);
    x86_function_append(function,
                        x86_sub(x86_operand_location_gpr(X86_GPR_RSP),
                                x86_operand_i32(stack_size)));

    // #NOTE: each basic block must include a terminator instruction which takes
    // it to the next basic block, or terminates the function itself.
    x86_function_append(function, x86_jmp(x86_operand_u32(1)));
    x86_function_target_block(function, previous_block);
}

void x86_function_footer(x86_Function *restrict function) {
    u32 block = x86_function_append_block(function);
    // #NOTE: each basic block must include a terminator instruction which takes
    // it to the next basic block, or terminates the function itself.
    x86_function_append(function, x86_jmp(x86_operand_u32(block)));

    // setup the function to insert instructions into the footer block
    x86_function_target_block(function, block);
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
    x86_function_append(function,
                        x86_mov(x86_operand_location_gpr(X86_GPR_RSP),
                                x86_operand_location_gpr(X86_GPR_RBP)));
    x86_function_append(function,
                        x86_pop(x86_operand_location_gpr(X86_GPR_RBP)));

    x86_function_append(function, x86_ret());
}
