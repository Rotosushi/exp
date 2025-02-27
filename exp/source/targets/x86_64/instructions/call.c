
#include <assert.h>

#include "targets/x86_64/instructions/call.h"
#include "targets/x86_64/mnemonic.h"

/*
 * prints the CALL (Near) x86_64 instruction to the buffer
 */

void x86_64_call(String *buffer, x86_64_Operand target) {
    print_x86_64_mnemonic(buffer, SV("call"), x86_64_operand_size(target));
    string_append(buffer, SV("\t"));
    print_x86_64_operand(buffer, target);
}
