/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "imr/frames.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

void frames_initialize(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    frames->length   = 0;
    frames->capacity = 0;
    frames->buffer   = nullptr;
}
void frames_terminate(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    deallocate(frames->buffer);
    frames_initialize(frames);
}

static bool frames_full(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    return (frames->length + 1) >= frames->capacity;
}

static void frames_grow(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    Growth32 g       = array_growth_u32(frames->capacity, sizeof(Frame));
    frames->buffer   = reallocate(frames->buffer, g.alloc_size);
    frames->capacity = g.new_capacity;
}

Frame *frames_push(Frames *frames, Function *function, u32 base) {
    EXP_ASSERT(frames != nullptr);
    if (frames_full(frames)) { frames_grow(frames); }
    Frame *frame = &frames->buffer[frames->length++];
    frame_initialize(frame, function, base);
    return frame;
}

void frames_pop(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    EXP_ASSERT(frames->length > 0);
    --frames->length;
}
