// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_TARGETS_X86_64_CODEGEN_INSTRUCTION_H
#define EXP_TARGETS_X86_64_CODEGEN_INSTRUCTION_H
#include "env/context.h"
#include "utility/result.h"

ExpResult x86_64_codegen_instruction(String *buffer, Instruction *I,
                                     Function *function,
                                     TranslationUnit *context);

#endif // EXP_TARGETS_X86_64_CODEGEN_INSTRUCTION_H
