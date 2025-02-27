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
#include <assert.h>

#include "backend/x64/context.h"
#include "backend/x64/function_body.h"
#include "intrinsics/size_of.h"
#include "utility/alloc.h"
#include "utility/panic.h"

void x64_formal_argument_list_create(x64_FormalArgumentList *args, u8 size) {
    assert(args != nullptr);
    args->size   = size;
    args->buffer = callocate(size, sizeof(x64_FormalArgument));
}

static void x64_formal_arguments_destroy(x64_FormalArgumentList *args) {
    args->size = 0;
    deallocate(args->buffer);
    args->buffer = nullptr;
}

x64_FormalArgument *x64_formal_argument_list_at(x64_FormalArgumentList *args,
                                                u8 idx) {
    assert(args != nullptr);
    assert(idx < args->size);
    return args->buffer + idx;
}

void x64_function_body_create(x64_FunctionBody *x64_body,
                              FunctionBody *body,
                              x64_Context *context) {
    assert(x64_body != nullptr);
    assert(body != nullptr);
    assert(context != nullptr);
    x64_formal_argument_list_create(&x64_body->arguments, body->arguments.size);
    x64_block_initialize(&x64_body->block);
    x64_allocator_initialize(&x64_body->allocator, body, context);
    x64_body->result         = nullptr;
    x64_Allocator *allocator = &x64_body->allocator;

    u8 scalar_argument_count = 0;

    if (type_is_scalar(body->return_type)) {
        x64_body->result = x64_allocator_allocate_result(
            allocator, x64_location_gpr(X64_GPR_RAX), body->return_type);
    } else {
        x64_Address result_address =
            x64_address_create(X64_GPR_RDI, X64_GPR_NONE, 1, 0);
        x64_body->result = x64_allocator_allocate_result(
            allocator,
            x64_location_address(
                x64_context_addresses_insert(context, result_address)),
            body->return_type);
        scalar_argument_count += 1;
    }

    i64 offset = 16;
    for (u8 i = 0; i < body->arguments.size; ++i) {
        FormalArgument *arg  = body->arguments.list + i;
        LocalVariable *local = function_body_locals_ssa(body, arg->ssa);

        if ((scalar_argument_count < 6) && type_is_scalar(local->type)) {
            x64_GPR gpr = x64_scalar_argument_gpr(scalar_argument_count++);
            x64_allocator_allocate_to_gpr(allocator, gpr, 0, local);
        } else {
            u64 argument_size = size_of(arg->type);
            assert(argument_size <= i64_MAX);

            x64_allocator_allocate_to_stack(allocator, offset, local);

            if (__builtin_add_overflow(offset, (i64)argument_size, &offset)) {
                PANIC("argument offset overflow");
            }
        }
    }
}

void x64_function_body_destroy(x64_FunctionBody *body) {
    assert(body != NULL);
    x64_formal_arguments_destroy(&body->arguments);
    x64_block_terminate(&body->block);
    x64_allocator_terminate(&body->allocator);
}
