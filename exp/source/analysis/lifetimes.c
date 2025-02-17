/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "analysis/lifetimes.h"
#include "env/context.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

typedef struct Subject {
    Function *function;
    Context *context;
} Subject;

static void subject_initialize(Subject *subject, Function *function,
                               Context *context) {
    EXP_ASSERT(subject != nullptr);
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    subject->function = function;
    subject->context  = context;
}

[[maybe_unused]] static bool validate_subject(Subject *subject) {
    if (subject == nullptr) return false;
    if (subject->function == nullptr) return false;
    if (subject->context == nullptr) return false;
    return true;
}

static void analyze_usage_of_operand(Operand operand, u32 block_index,
                                     Subject *subject) {
    EXP_ASSERT(validate_subject(subject));
    switch (operand.kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_local_at(subject->function, operand.data.ssa);
        EXP_ASSERT(local != nullptr);
        if (block_index > local->lifetime.last_use) {
            local_update_last_use(local, block_index);
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *constant =
            context_constants_at(subject->context, operand.data.constant);
        EXP_ASSERT(constant != nullptr);
        if (constant->kind != VALUE_KIND_TUPLE) break;
        Tuple *tuple = &constant->tuple;
        for (u64 index = 0; index < tuple->size; ++index) {
            Operand element = tuple->elements[index];
            analyze_usage_of_operand(element, block_index, subject);
        }
        break;
    }

    // #NOTE: a lifetime only makes sense w.r.t. local variables.
    //  A local can only be present within Operands directly, or
    //  within a Tuple. becuase Tuples are composed of Operands.
    //  thus nothing needs to be done for any other kind of Operand.
    default: break;
    }
}

static void analyze_first_use(Instruction *instruction, u32 block_index,
                              Subject *subject) {
    EXP_ASSERT(validate_subject(subject));
    switch (instruction->A.kind) {
    case OPERAND_KIND_SSA: {
        Local *local =
            function_local_at(subject->function, instruction->A.data.ssa);
        EXP_ASSERT(local != nullptr);
        local_update_first_use(local, block_index);
        break;
    }

    // #NOTE: the same reasoning applies for A position operands as for other
    //  operands, in addition, position A is exclusively used for Locals. as we
    //  do not have global variables.
    default: break;
    }
}

/*
[[maybe_unused]] static void analyze_usage_of_A(Instruction *instruction,
                                                u64 block_index,
                                                Subject *subject) {
    EXP_ASSERT(validate_subject(subject));
    analyze_usage_of_operand(
        instruction->B_kind, instruction->B_data, block_index, subject);
}
*/

static void analyze_usage_of_AB(Instruction *instruction, u32 block_index,
                                Subject *subject) {
    EXP_ASSERT(validate_subject(subject));
    analyze_first_use(instruction, block_index, subject);
    analyze_usage_of_operand(instruction->B, block_index, subject);
}

static void analyze_usage_of_ABC(Instruction *instruction, u32 block_index,
                                 Subject *subject) {
    EXP_ASSERT(validate_subject(subject));
    analyze_first_use(instruction, block_index, subject);
    analyze_usage_of_operand(instruction->B, block_index, subject);
    analyze_usage_of_operand(instruction->C, block_index, subject);
}

static void analyze_usage_of_instruction(Instruction *instruction,
                                         u32 block_index, Subject *subject) {
    EXP_ASSERT(validate_subject(subject));
    switch (instruction->opcode) {
    case OPCODE_RET:
        analyze_usage_of_AB(instruction, block_index, subject);
        break;
    case OPCODE_CALL:
        analyze_usage_of_ABC(instruction, block_index, subject);
        break;
    case OPCODE_NEG:
        analyze_usage_of_AB(instruction, block_index, subject);
        break;
    case OPCODE_DOT:
        analyze_usage_of_ABC(instruction, block_index, subject);
        break;
    case OPCODE_ADD:
        analyze_usage_of_ABC(instruction, block_index, subject);
        break;
    case OPCODE_SUB:
        analyze_usage_of_ABC(instruction, block_index, subject);
        break;
    case OPCODE_MUL:
        analyze_usage_of_ABC(instruction, block_index, subject);
        break;
    case OPCODE_DIV:
        analyze_usage_of_ABC(instruction, block_index, subject);
        break;
    case OPCODE_MOD:
        analyze_usage_of_ABC(instruction, block_index, subject);
        break;

    default: EXP_UNREACHABLE();
    }
}

static void analyze_usage_of_block(Subject *subject) {
    EXP_ASSERT(validate_subject(subject));
    Block *block = &subject->function->block;
    for (u32 index = 0; index < block->length; ++index) {
        analyze_usage_of_instruction(block->buffer + index, index, subject);
    }
}

void analyze_lifetimes_of_locals(Function *function, Context *context) {
    Subject subject;
    subject_initialize(&subject, function, context);
    analyze_usage_of_block(&subject);
}
