// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_FRAMES_H
#define EXP_IMR_FRAMES_H

#include "imr/frame.h"

typedef struct Frames {
    u32 length;
    u32 capacity;
    Frame *buffer;
} Frames;

void frames_initialize(Frames *frames);
void frames_terminate(Frames *frames);

Frame *frames_push(Frames *frames, Function *function, u32 base);
void frames_pop(Frames *frames);

#endif // !EXP_IMR_FRAMES_H
