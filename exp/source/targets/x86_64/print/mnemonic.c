/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "targets/x86_64/print/mnemonic.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

static bool valid_operand_size(u8 size) {
    return (size == 1) || (size == 2) || (size == 4) || (size == 8);
}

void print_x86_64_mnemonic(String *buffer, StringView mnemonic, u8 size) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(valid_operand_size(size));
    // #NOTE: x86 instructions can be postfixed with a size mnemonic
    //  to allow the assembler to emit more explicit instructions.
    //  the mnemonic makes explicit the size of the operands involved.
    //  technically it is only required when the size cannot be inferred
    //  by the operands themselves (such as on operand being a 64 bit register)
    //  however it is good practice to be as explicit as possible.
    //  'b' -> 1 byte
    //  'w' -> 2 byte
    //  'l' -> 4 byte
    //  'q' -> 8 byte
    //  #NOTE: floating point types are not supported yet, but they have their
    //   own size mnemonics
    //   's' -> f32
    //   'l' -> f64
    //   't' -> f80
    string_append(buffer, SV("  "));
    string_append(buffer, mnemonic);
    switch (size) {
    case 1:  string_append(buffer, SV("b")); break;
    case 2:  string_append(buffer, SV("w")); break;
    case 4:  string_append(buffer, SV("l")); break;
    case 8:  string_append(buffer, SV("q")); break;
    default: EXP_UNREACHABLE();
    }
    string_append(buffer, SV("\t"));
}
