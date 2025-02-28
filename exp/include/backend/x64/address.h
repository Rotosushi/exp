
#ifndef EXP_BACKEND_X64_ADDRESS_H
#define EXP_BACKEND_X64_ADDRESS_H

#include "backend/x64/registers.h"

typedef struct x64_Address {
    x64_GPR base;
    x64_GPR index;
    u8 scale;
    i64 offset;
} x64_Address;

x64_Address
x64_address_create(x64_GPR base, x64_GPR index, u8 scale, i64 offset);

bool x64_address_equality(x64_Address A, x64_Address B);

#endif // EXP_BACKEND_X64_ADDRESS_H
