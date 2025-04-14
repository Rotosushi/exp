
#ifndef EXP_BACKEND_X64_ADDRESS_H
#define EXP_BACKEND_X64_ADDRESS_H

#include "codegen/x86/imr/registers.h"

typedef struct x64_Address {
    x86_64_GPR base;
    x86_64_GPR index;
    u8         scale;
    bool       has_index;
    i64        offset;
} x64_Address;

x64_Address x64_address_create(x86_64_GPR base, i64 offset);

x64_Address x64_address_create_indexed(x86_64_GPR base,
                                       x86_64_GPR index,
                                       u8         scale,
                                       i64        offset);

/**
 * @brief compares two addresses for equality
 *
 * @note this does a lexicographic comparison,
 * and we do not use the values stored within the registers
 * (because we cannot) to confirm true equality. Thus this
 * method should be used with caution. if at all.
 */
bool x64_address_equality(x64_Address A, x64_Address B);

#endif // EXP_BACKEND_X64_ADDRESS_H
