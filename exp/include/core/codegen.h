// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
/**
 * @file core/codegen.h
 */
#ifndef EXP_CODEGEN_CODEGEN_H
#define EXP_CODEGEN_CODEGEN_H
#include "env/context.h"

/**
 * @brief Converts the IR in the given context into x86_64 assembly.
 * writes the assembly to the file specified by context_assembly_path(context).
 */
void codegen(Context *restrict context);

#endif // !EXP_CODEGEN_CODEGEN_H
