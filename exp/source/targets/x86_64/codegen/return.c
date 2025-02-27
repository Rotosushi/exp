

#include <assert.h>

#include "intrinsics/type_of.h"
#include "targets/x86_64/codegen/return.h"
#include "targets/x86_64/instructions/ret.h"
#include "utility/unreachable.h"

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
    assert(instruction != nullptr);
    assert(function != nullptr);
    assert(context != nullptr);

    assert(instruction->A_kind == OPERAND_KIND_SSA);
    Local *result = function_local_at(function, instruction->A_data.ssa);
    assert(result != nullptr);

    switch (instruction->B_kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_local_at(function, instruction->B_data.ssa);
        assert(local != nullptr);

        break;
    }

    case OPERAND_KIND_I32: {

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

    print_x86_64_ret(buffer);
    return EXP_SUCCESS;
}
