
#include <assert.h>

#include "targets/x86_64/instructions/ret.h"

/*
 *  prints the RET (near) x86 instruction.
 *  #NOTE: this instruction is one of only a few that has
 *    no need for an explicit size mnemonic when using as.
 */

void x86_64_ret(String *buffer) {
    assert(buffer != nullptr);
    string_append(buffer, SV("  ret"));
}
