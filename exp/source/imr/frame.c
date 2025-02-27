/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
