
#include <assert.h>

#include "analysis/allocation.h"
#include "env/context.h"
#include "utility/unreachable.h"

typedef struct Subject {
    LocalAllocator *allocator;
    FunctionBody *function;
    Context *context;
} Subject;

static void
subject_initialize(Subject *subject, FunctionBody *function, Context *context) {
    assert(subject != nullptr);
    assert(function != nullptr);
    assert(context != nullptr);
    subject->allocator = &function->allocator;
    subject->function  = function;
    subject->context   = context;
}

[[maybe_unused]] bool validate_subject(Subject *subject) {
    if (subject == nullptr) return false;
    if (subject->function == nullptr) return false;
    if (subject->context == nullptr) return false;
    return true;
}

// #NOTE: each local goes into a register sequentially,
//  unless it cannot fit into a register, then it is spilled
//  to the stack. if a local's lifetime is passed, then we
//  consider it's register open again, and the next local
//  allocated to a register is allocated to that register.

Local *local_from_operand_A(Instruction I, Subject *subject) {
    assert(validate_subject(subject));
    switch (I.A_kind) {
    case OPERAND_KIND_SSA:
        return function_body_local_at(subject->function, I.A_data.ssa);

    case OPERAND_KIND_LABEL:
        StringView name = constant_string_to_view(I.A_data.label);
        return function_body_local_at_name(subject->function, name);

    default: EXP_UNREACHABLE();
    }
}

void allocate_A(Instruction I, u64 block_index, Subject *subject) {
    assert(validate_subject(subject));
    Local *local = local_from_operand_A(I, subject);
    function_body_allocate_local(subject->function, local, block_index);
}

void allocate_locals(FunctionBody *function, Context *context) {
    assert(function != nullptr);
    assert(context != nullptr);
    Subject subject;
    subject_initialize(&subject, function, context);
    Block *block = &function->block;

    for (u64 index = 0; index < block->length; ++index) {
        allocate_A(block->buffer[index], index, &subject);
    }
}
