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

/**
 * @file imr/frame.c
 */

#include "imr/frame.h"
#include "utility/assert.h"

void frame_initialize(Frame *frame, Function *function, u32 base) {
    EXP_ASSERT(frame != nullptr);
    EXP_ASSERT(function != nullptr);

    frame->function = function;
    frame->ip       = function->block.buffer;
    frame->base     = base;
    frame->length   = function->arguments.length;
}

void frame_extend(Frame *frame, u32 n) {
    EXP_ASSERT(frame != nullptr);
    frame->length += n;
}
