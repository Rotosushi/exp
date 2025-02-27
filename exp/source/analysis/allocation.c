
#include <assert.h>

#include "analysis/allocation.h"
#include "env/context.h"
// #include "utility/unreachable.h"

typedef struct Subject {
    //    LocalAllocator *allocator;
    Function *function;
    Context *context;
} Subject;

static void
subject_initialize(Subject *subject, Function *function, Context *context) {
    assert(subject != nullptr);
    assert(function != nullptr);
    assert(context != nullptr);
    //  subject->allocator = &function->allocator;
    subject->function = function;
    subject->context  = context;
}

[[maybe_unused]] bool nonnull_subject(Subject *subject) {
    if (subject == nullptr) return false;
    if (subject->function == nullptr) return false;
    if (subject->context == nullptr) return false;
    return true;
}

Local *local_from_operand_A(Instruction *instruction, Subject *subject) {
    assert(instruction != nullptr);
    assert(nonnull_subject(subject));
    assert(instruction->A_kind == OPERAND_KIND_SSA);
    return function_local_at(subject->function, instruction->A_data.ssa);
}

static void
allocate_A(Instruction *instruction, u32 block_index, Subject *subject) {
    assert(instruction != nullptr);
    assert(nonnull_subject(subject));
    Local *local = local_from_operand_A(instruction, subject);
    function_allocate_local(subject->function, local, block_index);
}

static void allocate_instruction(Instruction *instruction,
                                 u32 block_index,
                                 Subject *subject) {
    assert(instruction != nullptr);
    assert(nonnull_subject(subject));
    allocate_A(instruction, block_index, subject);
}

static void allocate_formal_arguments(Subject *subject) {
    assert(nonnull_subject(subject));
    FormalArgumentList *formal_arguments = &subject->function->arguments;
    for (u8 index = 0; index < formal_arguments->size; ++index) {
        Local *argument = formal_arguments->list[index];
        assert(argument != nullptr);
        function_allocate_local(subject->function, argument, 0);
    }
}

void allocate_locals(Function *function, Context *context) {
    assert(function != nullptr);
    assert(context != nullptr);
    Subject subject;
    subject_initialize(&subject, function, context);

    allocate_formal_arguments(&subject);

    Block *block = &function->block;
    for (u32 index = 0; index < block->length; ++index) {
        allocate_instruction(block->buffer + index, index, &subject);
    }
}
