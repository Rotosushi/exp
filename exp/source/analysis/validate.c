/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "analysis/validate.h"
#include "env/context.h"
#include "intrinsics/type_of.h"
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

[[maybe_unused]] bool nonnull_subject(Subject *subject) {
    if (subject == nullptr) return false;
    if (subject->function == nullptr) return false;
    if (subject->context == nullptr) return false;
    return true;
}

static ExpResult validate_local(Local *local, Subject *subject) {
    EXP_ASSERT(local != nullptr);
    EXP_ASSERT(nonnull_subject(subject));
    if (local->type == nullptr) return EXP_FAILURE;
    Lifetime lifetime = local->lifetime;
    Lifetime bounds   = lifetime_construct(0, subject->function->block.length);
    if (lifetime.last_use < lifetime.first_use) return EXP_FAILURE;
    if (lifetime.first_use < bounds.first_use) return EXP_FAILURE;
    if (lifetime.last_use >= bounds.last_use) return EXP_FAILURE;
    return EXP_SUCCESS;
}

static ExpResult validate_locals(Subject *subject) {
    EXP_ASSERT(nonnull_subject(subject));
    Locals *locals = &subject->function->locals;
    for (u32 index = 0; index < locals->length; ++index) {
        if (validate_local(locals->buffer + index, subject) != EXP_SUCCESS) {
            return EXP_FAILURE;
        }
    }
    return EXP_SUCCESS;
}

// check that each declaration of a local occurs only once
//  we could use a dynamic bitset to store <have we seen this local declared
//  before? (yes/no)> we could store their pointer value in an array and use
//  nullptr to sigal (yes/no) can we turn ansering the question into a constant
//  time operation over a dynamic vector?
// check that each use of a local occurs after it's declaration
// check that each use of a local is typesafe
// check that each use of a value is typesafe
// check that each use of a label is typesafe
static ExpResult validate_operand(Operand operand, u32 block_index,
                                  Subject *subject);

static ExpResult validate_tuple(Tuple *tuple, u32 block_index,
                                Subject *subject) {
    for (u32 index = 0; index < tuple->size; ++index) {
        Operand element = tuple->elements[index];
        if (validate_operand(element, block_index, subject) != EXP_SUCCESS) {
            return EXP_FAILURE;
        }
    }
    return EXP_SUCCESS;
}

static ExpResult validate_constant(Value *value, u32 block_index,
                                   Subject *subject) {
    EXP_ASSERT(value != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break;
    case VALUE_KIND_SCALAR:        break;
    case VALUE_KIND_TUPLE:
        return validate_tuple(&value->tuple, block_index, subject);
    default: EXP_UNREACHABLE();
    }

    return EXP_SUCCESS;
}

static ExpResult validate_operand(Operand operand, u32 block_index,
                                  Subject *subject) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_local_at(subject->function, operand.data.ssa);
        EXP_ASSERT(local != nullptr);
        if (validate_local(local, subject) != EXP_SUCCESS) {
            return EXP_FAILURE;
        }
        Lifetime lifetime = local->lifetime;
        if (lifetime.first_use > block_index) { return EXP_FAILURE; }
        if (block_index > lifetime.last_use) { return EXP_FAILURE; }
        break;
    }

    case OPERAND_KIND_SCALAR: {
        break;
    }

    case OPERAND_KIND_LABEL: {
        StringView label =
            context_labels_at(subject->context, operand.data.label);
        Symbol *symbol = context_symbol_table_at(subject->context, label);
        EXP_ASSERT(symbol != nullptr);
        EXP_ASSERT(symbol->type != nullptr);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *constant =
            context_constants_at(subject->context, operand.data.constant);
        EXP_ASSERT(constant != nullptr);
        if (validate_constant(constant, block_index, subject) != EXP_SUCCESS) {
            return EXP_FAILURE;
        }
        break;
    }

    default: EXP_UNREACHABLE();
    }

    return EXP_SUCCESS;
}

static ExpResult validate_A(Instruction *instruction, u32 block_index,
                            Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_operand(instruction->A, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    switch (instruction->A.kind) {
    case OPERAND_KIND_SSA: {
        Local *ssa =
            function_local_at(subject->function, instruction->A.data.ssa);
        EXP_ASSERT(ssa != nullptr);
        if (validate_local(ssa, subject) != EXP_SUCCESS) { return EXP_FAILURE; }
        // #NOTE: in addition to catching the mismatch between the declaration
        //  of a particular local and the lifetime of that local, this check
        //  also ensures that each ssa only appears in position A once in the
        //  block. for the local to appear a second time implies appearing after
        //  the first time, thus block_index would have to be greater than
        //  first_use. if it's less than, this is also an error, which is why we
        //  use != over <
        if (ssa->lifetime.first_use != block_index) { return EXP_FAILURE; }
        break;
    }

    // #NOTE: scalar operands are disallowed in position A.
    //  though I might use a u32 there in jump instructions
    case OPERAND_KIND_SCALAR:
    // #NOTE: constants are disallowed in position A.
    //  though a Tuple in position A could be used for
    //  multiple declaration statements.
    //  const (a, b) = f();
    case OPERAND_KIND_CONSTANT:
    // #NOTE: labels are disallowed, they are currently used
    //  for global access. so this would only be valid for
    //  a global store operation on a global variable. which
    //  is unsupported currently.
    case OPERAND_KIND_LABEL: {
        return EXP_FAILURE;
    }
    }

    return EXP_SUCCESS;
}

static ExpResult validate_AB(Instruction *instruction, u32 block_index,
                             Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_A(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    if (validate_operand(instruction->B, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    return EXP_SUCCESS;
}

static ExpResult validate_ABC(Instruction *instruction, u32 block_index,
                              Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_AB(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    if (validate_operand(instruction->C, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    return EXP_SUCCESS;
}

static ExpResult validate_load(Instruction *instruction, u32 block_index,
                               Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(instruction->opcode == OPCODE_LOAD);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_AB(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    Type const *target_type =
        type_of_operand(instruction->A, subject->function, subject->context);
    EXP_ASSERT(target_type != nullptr);
    Type const *source_type =
        type_of_operand(instruction->B, subject->function, subject->context);
    EXP_ASSERT(source_type != nullptr);
    if (!type_equal(target_type, source_type)) { return EXP_FAILURE; }
    return EXP_SUCCESS;
}

static ExpResult validate_return(Instruction *instruction, u32 block_index,
                                 Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(instruction->opcode == OPCODE_RET);

    if (validate_AB(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    Type const *A_type =
        type_of_operand(instruction->A, subject->function, subject->context);
    EXP_ASSERT(A_type != nullptr);

    Type const *B_type =
        type_of_operand(instruction->B, subject->function, subject->context);
    EXP_ASSERT(B_type != nullptr);

    if (!type_equal(A_type, B_type)) { return EXP_FAILURE; }

    Type const *return_type = subject->function->return_type;
    EXP_ASSERT(return_type != nullptr);

    if (!type_equal(A_type, return_type)) { return EXP_FAILURE; }
    return EXP_SUCCESS;
}

static ExpResult validate_call(Instruction *instruction, u32 block_index,
                               Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_ABC(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    EXP_ASSERT(instruction->A.kind == OPERAND_KIND_SSA);
    Local *ssa = function_local_at(subject->function, instruction->A.data.ssa);
    EXP_ASSERT(ssa != nullptr);

    Type const *B_type =
        type_of_operand(instruction->B, subject->function, subject->context);
    EXP_ASSERT(B_type != nullptr);

    // #TODO: some sort of "callable" interface could be checked against here
    //  instead of being hardcoded.
    if (B_type->kind != TYPE_KIND_FUNCTION) { return EXP_FAILURE; }
    FunctionType const *callee_type   = &B_type->function_type;
    TupleType const *formal_arguments = &callee_type->argument_types;

    Type const *C_type =
        type_of_operand(instruction->C, subject->function, subject->context);
    EXP_ASSERT(C_type != nullptr);

    if (C_type->kind != TYPE_KIND_TUPLE) { return EXP_FAILURE; }
    TupleType const *actual_arguments = &C_type->tuple_type;

    if (formal_arguments->count != actual_arguments->count) {
        return EXP_FAILURE;
    }

    for (u64 index = 0; index < formal_arguments->count; ++index) {
        Type const *formal = formal_arguments->types[index];
        EXP_ASSERT(formal != nullptr);

        Type const *actual = actual_arguments->types[index];
        EXP_ASSERT(actual != nullptr);

        if (!type_equal(formal, actual)) { return EXP_FAILURE; }
    }

    Type const *result_type = callee_type->return_type;
    EXP_ASSERT(result_type != nullptr);

    if (!type_equal(ssa->type, result_type)) { return EXP_FAILURE; }

    return EXP_SUCCESS;
}

static ExpResult validate_unop(Type const *return_type,
                               Type const *argument_type,
                               Instruction *instruction, u32 block_index,
                               Subject *subject) {
    EXP_ASSERT(return_type != nullptr);
    EXP_ASSERT(argument_type != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_AB(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    EXP_ASSERT(instruction->A.kind == OPERAND_KIND_SSA);
    Local *ssa = function_local_at(subject->function, instruction->A.data.ssa);
    EXP_ASSERT(ssa->type != nullptr);

    if (!type_equal(return_type, ssa->type)) { return EXP_FAILURE; }

    Type const *B_type =
        type_of_operand(instruction->B, subject->function, subject->context);
    EXP_ASSERT(B_type != nullptr);

    if (!type_equal(argument_type, B_type)) { return EXP_FAILURE; }

    return EXP_SUCCESS;
}

static ExpResult validate_negate(Instruction *instruction, u32 block_index,
                                 Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Type const *i32_type = context_i32_type(subject->context);
    return validate_unop(i32_type, i32_type, instruction, block_index, subject);
}

static ExpResult validate_dot(Instruction *instruction, u32 block_index,
                              Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_ABC(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    EXP_ASSERT(instruction->A.kind == OPERAND_KIND_SSA);
    Local *ssa = function_local_at(subject->function, instruction->A.data.ssa);
    EXP_ASSERT(ssa->type != nullptr);

    if (instruction->B.kind != OPERAND_KIND_SCALAR) { return EXP_FAILURE; }
    if (!scalar_is_index(instruction->B.data.scalar)) { return EXP_FAILURE; }
    u64 index = scalar_index(instruction->B.data.scalar);

    Type const *C_type =
        type_of_operand(instruction->C, subject->function, subject->context);
    EXP_ASSERT(C_type != nullptr);

    if (C_type->kind != TYPE_KIND_TUPLE) { return EXP_FAILURE; }
    TupleType const *tuple = &C_type->tuple_type;

    if (index < 0) { return EXP_FAILURE; }
    if ((u32)index >= tuple->count) { return EXP_FAILURE; }

    Type const *element_type = tuple->types[index];
    EXP_ASSERT(element_type != nullptr);

    if (!type_equal(ssa->type, element_type)) { return EXP_FAILURE; }

    return EXP_SUCCESS;
}

static ExpResult validate_binop(Type const *return_type, Type const *left_type,
                                Type const *right_type,
                                Instruction *instruction, u32 block_index,
                                Subject *subject) {
    EXP_ASSERT(return_type != nullptr);
    EXP_ASSERT(left_type != nullptr);
    EXP_ASSERT(right_type != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    if (validate_ABC(instruction, block_index, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    EXP_ASSERT(instruction->A.kind == OPERAND_KIND_SSA);
    Local *ssa = function_local_at(subject->function, instruction->A.data.ssa);
    EXP_ASSERT(ssa->type != nullptr);

    if (!type_equal(return_type, ssa->type)) { return EXP_FAILURE; }

    Type const *B_type =
        type_of_operand(instruction->B, subject->function, subject->context);
    EXP_ASSERT(B_type != nullptr);

    if (!type_equal(left_type, B_type)) { return EXP_FAILURE; }

    Type const *C_type =
        type_of_operand(instruction->C, subject->function, subject->context);
    EXP_ASSERT(C_type != nullptr);

    if (!type_equal(right_type, C_type)) { return EXP_FAILURE; }

    return EXP_SUCCESS;
}

static ExpResult validate_arithmetic_binop(Instruction *instruction,
                                           u32 block_index, Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Type const *B_type =
        type_of_operand(instruction->B, subject->function, subject->context);
    EXP_ASSERT(B_type != nullptr);
    if (!type_is_arithmetic(B_type)) { return EXP_FAILURE; }

    return validate_binop(B_type, B_type, B_type, instruction, block_index,
                          subject);
}

static ExpResult validate_instruction(Instruction *instruction, u32 block_index,
                                      Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    switch (instruction->opcode) {
    case OPCODE_LOAD: return validate_load(instruction, block_index, subject);
    case OPCODE_RET:  return validate_return(instruction, block_index, subject);
    case OPCODE_CALL: return validate_call(instruction, block_index, subject);
    case OPCODE_NEG:  return validate_negate(instruction, block_index, subject);
    case OPCODE_DOT:  return validate_dot(instruction, block_index, subject);
    case OPCODE_ADD:
        return validate_arithmetic_binop(instruction, block_index, subject);
    case OPCODE_SUB:
        return validate_arithmetic_binop(instruction, block_index, subject);
    case OPCODE_MUL:
        return validate_arithmetic_binop(instruction, block_index, subject);
    case OPCODE_DIV:
        return validate_arithmetic_binop(instruction, block_index, subject);
    case OPCODE_MOD:
        return validate_arithmetic_binop(instruction, block_index, subject);
    default: EXP_UNREACHABLE();
    }

    return EXP_SUCCESS;
}

static ExpResult validate_block(Subject *subject) {
    EXP_ASSERT(nonnull_subject(subject));

    Block *block = &subject->function->block;
    for (u32 index = 0; index < block->length; ++index) {
        if (validate_instruction(block->buffer + index, index, subject) !=
            EXP_SUCCESS) {
            return EXP_FAILURE;
        }
    }

    return EXP_SUCCESS;
}

ExpResult validate_function(Function *function, Context *context) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);

    Subject subject;
    subject_initialize(&subject, function, context);
    if (validate_locals(&subject) != EXP_SUCCESS) { return EXP_FAILURE; }
    if (validate_block(&subject) != EXP_SUCCESS) { return EXP_FAILURE; }

    return EXP_SUCCESS;
}
