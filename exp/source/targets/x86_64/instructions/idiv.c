

#include "targets/x86_64/instructions/idiv.h"
#include "targets/x86_64/mnemonic.h"

void x86_64_idiv(String *buffer, x86_64_Operand target) {
    print_x86_64_mnemonic(buffer, SV("idiv"), x86_64_operand_size(target));
    string_append(buffer, SV("\t"));
    print_x86_64_operand(buffer, target);
}
