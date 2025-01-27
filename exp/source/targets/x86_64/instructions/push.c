

#include <assert.h>

#include "targets/x86_64/instructions/push.h"
#include "targets/x86_64/mnemonic.h"

void x86_64_push(String *buffer, x86_64_Operand target) {
    assert(buffer != nullptr);
    print_x86_64_mnemonic(buffer, SV("push"), x86_64_operand_size(target));
    string_append(buffer, SV("\t"));
    print_x86_64_operand(buffer, target);
}
