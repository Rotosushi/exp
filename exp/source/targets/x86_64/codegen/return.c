

#include <assert.h>

#include "intrinsics/type_of.h"
#include "targets/x86_64/codegen/return.h"
#include "utility/unreachable.h"

[[maybe_unused]] static bool validate_return_instruction(
    Instruction *instruction, Function *function, Context *context) {
    if (instruction == nullptr) return false;
    if (instruction->opcode != OPCODE_RETURN) return false;
    if (instruction->A_kind != OPERAND_KIND_SSA) return false;
    if (function == nullptr) return false;
    if (function->return_type == nullptr) return false;
    if (context == nullptr) return false;

    Type const *B_type = type_of_operand(
        instruction->B_kind, instruction->B_data, function, context);
    return type_equality(B_type, function->return_type);
}

/*
 * #NOTE: the return instruction could be refactored to AB type,
 *  where we use A to allocate the return slot. However, this immediately
 *  opens up the fact that multiple return expressions must utilize a
 *  phi node to resolve the assignment. strictly speaking. as of now,
 *  the return slot is implicit to the function itself.
 */

ExpResult x86_64_codegen_return(String *buffer,
                                Instruction *instruction,
                                Function *function,
                                Context *context) {
    assert(buffer != nullptr);
    assert(validate_return_instruction(instruction, function, context));

    Local *result = instruction->A_data.ssa;
    assert(result != nullptr);

    switch (instruction->B_kind) {
    case OPERAND_KIND_SSA: {
        Local *local = instruction->B_data.ssa;
        assert(local != nullptr);

        break;
    }

    case OPERAND_KIND_I64: {

        break;
    }

    case OPERAND_KIND_CONSTANT: {

        break;
    }

    case OPERAND_KIND_LABEL: {
        EXP_UNREACHABLE();
        break;
    }

    default: EXP_UNREACHABLE();
    }

    return EXP_SUCCESS;
}
