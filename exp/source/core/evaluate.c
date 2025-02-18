/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "core/evaluate.h"
#include "utility/assert.h"
#include "utility/result.h"
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

[[maybe_unused]] static bool nonull_subject(Subject *subject) {
    if (subject == nullptr) { return false; }
    if (subject->function == nullptr) { return false; }
    if (subject->context == nullptr) { return false; }
    return true;
}

static ExpResult evaluate_function(Subject *subject);
static ExpResult evaluate_instruction(Instruction instruction,
                                      Subject *subject);

ExpResult evaluate(Context *context) {
    EXP_ASSERT(context != nullptr);

    SymbolTable *symbol_table = &context->symbol_table;
    for (u64 index = 0; index < symbol_table->capacity; ++index) {
        Symbol *symbol = symbol_table->elements[index];
        if (symbol == nullptr) { continue; }
        Function *function = &symbol->function_body;
        Subject subject;
        subject_initialize(&subject, function, context);
        if (evaluate_function(&subject) != EXP_SUCCESS) { return EXP_FAILURE; }
    }
    return EXP_SUCCESS;
}

static ExpResult evaluate_function(Subject *subject) {
    EXP_ASSERT(nonull_subject(subject));

    Block *block = &subject->function->block;
    for (u32 index = 0; index < block->length; ++index) {
        if (evaluate_instruction(block->buffer[index], subject) !=
            EXP_SUCCESS) {
            return EXP_FAILURE;
        }
    }
    return EXP_SUCCESS;
}

static ExpResult evaluate_return(Instruction instruction, Subject *subject);
static ExpResult evaluate_call(Instruction instruction, Subject *subject);
static ExpResult evaluate_dot(Instruction instruction, Subject *subject);
static ExpResult evaluate_load(Instruction instruction, Subject *subject);
static ExpResult evaluate_neg(Instruction instruction, Subject *subject);
static ExpResult evaluate_add(Instruction instruction, Subject *subject);
static ExpResult evaluate_sub(Instruction instruction, Subject *subject);
static ExpResult evaluate_mul(Instruction instruction, Subject *subject);
static ExpResult evaluate_div(Instruction instruction, Subject *subject);
static ExpResult evaluate_mod(Instruction instruction, Subject *subject);

static ExpResult evaluate_instruction(Instruction instruction,
                                      Subject *subject) {
    EXP_ASSERT(nonull_subject(subject));
    switch (instruction.opcode) {
    case OPCODE_RET:  return evaluate_return(instruction, subject);
    case OPCODE_CALL: return evaluate_call(instruction, subject);
    case OPCODE_DOT:  return evaluate_dot(instruction, subject);
    case OPCODE_LOAD: return evaluate_load(instruction, subject);
    case OPCODE_NEG:  return evaluate_neg(instruction, subject);
    case OPCODE_ADD:  return evaluate_add(instruction, subject);
    case OPCODE_SUB:  return evaluate_sub(instruction, subject);
    case OPCODE_MUL:  return evaluate_mul(instruction, subject);
    case OPCODE_DIV:  return evaluate_div(instruction, subject);
    case OPCODE_MOD:  return evaluate_mod(instruction, subject);
    default:          EXP_UNREACHABLE();
    }
}

static ExpResult evaluate_return(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_call(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_dot(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_load(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_neg(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_add(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_sub(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_mul(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_div(Instruction instruction, Subject *subject) {}
static ExpResult evaluate_mod(Instruction instruction, Subject *subject) {}
