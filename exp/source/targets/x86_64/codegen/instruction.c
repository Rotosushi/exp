/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include <assert.h>

#include "targets/x86_64/codegen/add.h"
#include "targets/x86_64/codegen/call.h"
#include "targets/x86_64/codegen/divide.h"
#include "targets/x86_64/codegen/dot.h"
#include "targets/x86_64/codegen/instruction.h"
#include "targets/x86_64/codegen/load.h"
#include "targets/x86_64/codegen/modulus.h"
#include "targets/x86_64/codegen/multiply.h"
#include "targets/x86_64/codegen/negate.h"
#include "targets/x86_64/codegen/return.h"
#include "targets/x86_64/codegen/subtract.h"
#include "utility/unreachable.h"

ExpResult x86_64_codegen_instruction(String *buffer, Instruction *instruction,
                                     Function *function, Context *context) {
    assert(buffer != nullptr);
    assert(instruction != nullptr);
    assert(function != nullptr);
    assert(context != nullptr);

    switch (instruction->opcode) {
    case OPCODE_LOAD:
        return x86_64_codegen_load(buffer, instruction, function, context);

    case OPCODE_RETURN:
        return x86_64_codegen_return(buffer, instruction, function, context);

    case OPCODE_CALL:
        return x86_64_codegen_call(buffer, instruction, function, context);

    case OPCODE_NEGATE:
        return x86_64_codegen_negate(buffer, instruction, function, context);

    case OPCODE_DOT:
        return x86_64_codegen_dot(buffer, instruction, function, context);

    case OPCODE_ADD:
        return x86_64_codegen_add(buffer, instruction, function, context);

    case OPCODE_SUBTRACT:
        return x86_64_codegen_subtract(buffer, instruction, function, context);

    case OPCODE_MULTIPLY:
        return x86_64_codegen_multiply(buffer, instruction, function, context);

    case OPCODE_DIVIDE:
        return x86_64_codegen_divide(buffer, instruction, function, context);

    case OPCODE_MODULUS:
        return x86_64_codegen_modulus(buffer, instruction, function, context);

    default: EXP_UNREACHABLE();
    }
}
