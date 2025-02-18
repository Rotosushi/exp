/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "core/codegen.h"
#include "targets/ir/emit.h"
#include "utility/assert.h"

void codegen(TranslationUnit *context) {
    EXP_ASSERT(context != nullptr);

    if (context_emit_ir_assembly(context)) { emit_ir_assembly(context); }
}
