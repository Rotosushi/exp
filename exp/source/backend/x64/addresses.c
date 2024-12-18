
#include <assert.h>

#include "backend/x64/addresses.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

void x64_addresses_initialize(x64_Addresses *addresses) {
    assert(addresses != nullptr);
    addresses->count    = 0;
    addresses->capacity = 0;
    addresses->buffer   = nullptr;
}

void x64_addresses_terminate(x64_Addresses *addresses) {
    assert(addresses != nullptr);
    addresses->count    = 0;
    addresses->capacity = 0;
    deallocate(addresses->buffer);
    addresses->buffer = nullptr;
}

static bool x64_addresses_full(x64_Addresses *addresses) {
    assert(addresses != nullptr);
    return (addresses->count + 1) >= addresses->capacity;
}

static void x64_addresses_grow(x64_Addresses *addresses) {
    assert(addresses != nullptr);
    Growth64 g = array_growth_u64(addresses->capacity, sizeof(x64_Address));
    addresses->buffer   = reallocate(addresses->buffer, g.alloc_size);
    addresses->capacity = g.new_capacity;
}

u64 x64_addresses_insert(x64_Addresses *addresses, x64_Address address) {
    assert(addresses != nullptr);
    if (x64_addresses_full(addresses)) { x64_addresses_grow(addresses); }

    for (u64 i = 0; i < addresses->count; ++i) {
        x64_Address *cursor = addresses->buffer + i;
        if (x64_address_equality(*cursor, address)) { return i; }
    }

    u64 index                = addresses->count++;
    addresses->buffer[index] = address;
    return index;
}

x64_Address *x64_addresses_at(x64_Addresses *addresses, u64 index) {
    assert(addresses != nullptr);
    assert(index < addresses->count);
    return addresses->buffer + index;
}
