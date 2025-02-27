

#ifndef EXP_BACKEND_X64_ADDRESSES_H
#define EXP_BACKEND_X64_ADDRESSES_H

#include "backend/x64/address.h"

typedef struct x64_Addresses {
    u32 count;
    u32 capacity;
    x64_Address *buffer;
} x64_Addresses;

void x64_addresses_initialize(x64_Addresses *addresses);
void x64_addresses_terminate(x64_Addresses *addresses);

u32 x64_addresses_insert_unique(x64_Addresses *addresses, x64_Address address);
x64_Address *x64_addresses_at(x64_Addresses *addresses, u32 index);

#endif // EXP_BACKEND_X64_ADDRESSES_H
