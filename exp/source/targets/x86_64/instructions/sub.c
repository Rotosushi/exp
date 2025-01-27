
#include <assert.h>

#include "targets/x86_64/instructions/sub.h"
#include "targets/x86_64/mnemonic.h"

void x86_64_sub(String *buffer, x86_64_Operand target, x86_64_Operand source) {
    assert(buffer != nullptr);
    print_x86_64_mnemonic(buffer, SV("sub"), x86_64_operand_size(target));
    string_append(buffer, SV("\t"));
    print_x86_64_operand(buffer, source);
    string_append(buffer, SV(", "));
    print_x86_64_operand(buffer, target);
}
