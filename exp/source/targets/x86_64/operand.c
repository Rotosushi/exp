
#include <assert.h>

#include "targets/x86_64/operand.h"
#include "utility/unreachable.h"

static x86_64_Operand operand_create(x86_64_OperandKind kind,
                                     x86_64_OperandData data) {
    x86_64_Operand operand = {.kind = kind, .data = data};
    return operand;
}

x86_64_Operand x86_64_operand_gpr(x86_64_GPR gpr) {
    return operand_create(X86_64_OPERAND_KIND_GPR,
                          (x86_64_OperandData){.gpr = gpr});
}

x86_64_Operand x86_64_operand_address(x86_64_Address address) {
    return operand_create(X86_64_OPERAND_KIND_ADDRESS,
                          (x86_64_OperandData){.address = address});
}

x86_64_Operand x86_64_operand_i64(i64 i64_) {
    return operand_create(X86_64_OPERAND_KIND_I64,
                          (x86_64_OperandData){.i64_ = i64_});
}

x86_64_Operand x86_64_operand_label(ConstantString *label) {
    assert(label != nullptr);
    return operand_create(X86_64_OPERAND_KIND_LABEL,
                          (x86_64_OperandData){.label = label});
}

u8 x86_64_operand_size(x86_64_Operand operand) {
    switch (operand.kind) {
    case X86_64_OPERAND_KIND_GPR: return x86_64_gpr_size(operand.data.gpr);
    // #NOTE: an address (as in a pointer) is always 64 bits, but in the
    //  context of an x86_64 instruction, what is meant is the size of the
    //  target of the address. it isn't very robust, but we use the size of
    //  the base register as a stand-in here. for now.
    case X86_64_OPERAND_KIND_ADDRESS:
        return x86_64_gpr_size(operand.data.address.base);
    case X86_64_OPERAND_KIND_I64: return 8;
    // #NOTE: a label is effectively a pointer in assembly, so it's size is
    //  always 8.
    case X86_64_OPERAND_KIND_LABEL: return 8;
    default:                        EXP_UNREACHABLE();
    }
}

void print_x86_64_operand(String *buffer, x86_64_Operand operand) {
    assert(buffer != nullptr);
    switch (operand.kind) {
    case X86_64_OPERAND_KIND_GPR:
        print_x86_64_gpr(buffer, operand.data.gpr);
        break;
    case X86_64_OPERAND_KIND_ADDRESS:
        print_x86_64_address(buffer, operand.data.address);
        break;
    case X86_64_OPERAND_KIND_I64:
        string_append(buffer, SV("%"));
        string_append_i64(buffer, operand.data.i64_);
        break;
    case X86_64_OPERAND_KIND_LABEL:
        string_append(buffer, constant_string_to_view(operand.data.label));
        break;
    default: EXP_UNREACHABLE();
    }
}
