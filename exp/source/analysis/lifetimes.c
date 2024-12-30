
#include <assert.h>

#include "analysis/lifetimes.h"
#include "env/context.h"
#include "utility/unreachable.h"

typedef struct Subject {
    FunctionBody *function;
    Context *context;
} Subject;

static void
subject_initialize(Subject *subject, FunctionBody *function, Context *context) {
    assert(subject != nullptr);
    assert(function != nullptr);
    assert(context != nullptr);
    subject->function = function;
    subject->context  = context;
}

[[maybe_unused]] static bool validate_subject(Subject *subject) {
    if (subject == nullptr) return false;
    if (subject->function == nullptr) return false;
    if (subject->context == nullptr) return false;
    return true;
}

static void analyze_usage_of_operand(OperandKind kind,
                                     OperandData data,
                                     u64 block_index,
                                     Subject *subject) {
    assert(validate_subject(subject));
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_body_local_at(subject->function, data.ssa);
        assert(local != nullptr);
        if (block_index > local->lifetime.last_use) {
            local_update_last_use(local, block_index);
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *constant = data.constant;
        assert(constant != nullptr);
        if (constant->kind != VALUE_KIND_TUPLE) break;
        Tuple *tuple = &constant->tuple;
        for (u64 index = 0; index < tuple->size; ++index) {
            Operand element = tuple->elements[index];
            analyze_usage_of_operand(
                element.kind, element.data, block_index, subject);
        }
        break;
    }

    case OPERAND_KIND_LABEL: {
        StringView name = constant_string_to_view(data.label);
        Local *local    = function_body_local_at_name(subject->function, name);
        if (local == nullptr) break; // can validly be a global name
        if (block_index > local->lifetime.last_use) {
            local_update_last_use(local, block_index);
        }
        break;
    }

    // #NOTE: a lifetime only makes sense w.r.t. local variables.
    //  %ssa and label can only be present within Operands directly, or
    //  within a Tuple. becuase Tuples are composed of Operands.
    //  thus nothing needs to be done for any other kind of Operand.
    default: break;
    }
}

static void
analyze_first_use(Instruction I, u64 block_index, Subject *subject) {
    assert(validate_subject(subject));
    switch (I.A_kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_body_local_at(subject->function, I.A_data.ssa);
        assert(local != nullptr);
        local_update_first_use(local, block_index);
        break;
    }

    case OPERAND_KIND_LABEL: {
        StringView name = constant_string_to_view(I.A_data.label);
        Local *local    = function_body_local_at_name(subject->function, name);
        assert(local != nullptr);
        local_update_first_use(local, block_index);
        break;
    }

    // #NOTE: the same reasoning applies for A position operands
    default: break;
    }
}

static void
analyze_usage_of_B(Instruction I, u64 block_index, Subject *subject) {
    assert(validate_subject(subject));
    analyze_usage_of_operand(I.B_kind, I.B_data, block_index, subject);
}

static void
analyze_usage_of_AB(Instruction I, u64 block_index, Subject *subject) {
    assert(validate_subject(subject));
    analyze_first_use(I, block_index, subject);
    analyze_usage_of_operand(I.B_kind, I.B_data, block_index, subject);
}

static void
analyze_usage_of_ABC(Instruction I, u64 block_index, Subject *subject) {
    assert(validate_subject(subject));
    analyze_first_use(I, block_index, subject);
    analyze_usage_of_operand(I.B_kind, I.B_data, block_index, subject);
    analyze_usage_of_operand(I.C_kind, I.C_data, block_index, subject);
}

static void
analyze_usage_of_instruction(Instruction I, u64 block_index, Subject *subject) {
    assert(validate_subject(subject));
    switch (I.opcode) {
    case OPCODE_RETURN:   analyze_usage_of_B(I, block_index, subject); break;
    case OPCODE_CALL:     analyze_usage_of_ABC(I, block_index, subject); break;
    case OPCODE_NEGATE:   analyze_usage_of_AB(I, block_index, subject); break;
    case OPCODE_DOT:      analyze_usage_of_ABC(I, block_index, subject); break;
    case OPCODE_ADD:      analyze_usage_of_ABC(I, block_index, subject); break;
    case OPCODE_SUBTRACT: analyze_usage_of_ABC(I, block_index, subject); break;
    case OPCODE_MULTIPLY: analyze_usage_of_ABC(I, block_index, subject); break;
    case OPCODE_DIVIDE:   analyze_usage_of_ABC(I, block_index, subject); break;
    case OPCODE_MODULUS:  analyze_usage_of_ABC(I, block_index, subject); break;

    default: EXP_UNREACHABLE();
    }
}

static void analyze_usage_of_block(Subject *subject) {
    assert(validate_subject(subject));
    Block *block = &subject->function->block;
    for (u64 index = 0; index < block->length; ++index) {
        analyze_usage_of_instruction(block->buffer[index], index, subject);
    }
}

void analyze_lifetimes_of_locals(FunctionBody *function, Context *context) {
    Subject subject;
    subject_initialize(&subject, function, context);
    analyze_usage_of_block(&subject);
}
