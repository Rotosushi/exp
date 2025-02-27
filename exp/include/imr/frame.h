// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef EXP_IMR_FRAME_H
#define EXP_IMR_FRAME_H

#include "imr/function.h"
#include "imr/instruction.h"

typedef struct Frame {
    Function *function;
    Instruction *ip;
    u32 base;
    u32 length;
} Frame;

void frame_initialize(Frame *frame, Function *function, u32 base);
void frame_extend(Frame *frame, u32 n);

#endif // !EXP_IMR_FRAME_H
