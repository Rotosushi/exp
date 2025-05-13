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

#include "codegen/x86/env/context.h"
#include "support/allocation.h"
#include "support/assert.h"

void *x86_context_allocate() {
    x86_Context *context = callocate(1, sizeof(x86_Context));
    exp_assert_debug(context != NULL);
    x86_layouts_create(&context->layouts);
    return context;
}

void x86_context_deallocate(void *restrict context) {
    exp_assert(context != NULL);
    x86_Context *x86_context = (x86_Context *)context;
    x86_layouts_destroy(&x86_context->layouts);

    deallocate(context);
}

x86_Layout const *x86_context_layout_of(Context *restrict context,
                                        Type const *type) {
    exp_assert(context != NULL);
    exp_assert(type != NULL);
    x86_Context *x86_context =
        (x86_Context *)context_get_target_context(context);

    return x86_layouts_layout_of(&x86_context->layouts, type);
}
