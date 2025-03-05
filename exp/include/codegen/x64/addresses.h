

#ifndef EXP_BACKEND_X64_ADDRESSES_H
#define EXP_BACKEND_X64_ADDRESSES_H

#include "codegen/x64/address.h"

typedef struct x64_Addresses {
    u64 count;
    u64 capacity;
    x64_Address *buffer;
} x64_Addresses;

void x64_addresses_initialize(x64_Addresses *addresses);
void x64_addresses_terminate(x64_Addresses *addresses);

u64 x64_addresses_append(x64_Addresses *addresses, x64_Address address);
x64_Address *x64_addresses_at(x64_Addresses *addresses, u64 index);

#endif // EXP_BACKEND_X64_ADDRESSES_H
