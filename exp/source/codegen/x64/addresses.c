
#include <assert.h>

#include "codegen/x64/addresses.h"
#include "utility/allocation.h"
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
    Growth_u64 g = array_growth_u64(addresses->capacity, sizeof(x64_Address));
    addresses->buffer   = reallocate(addresses->buffer, g.alloc_size);
    addresses->capacity = g.new_capacity;
}

u64 x64_addresses_append(x64_Addresses *addresses, x64_Address address) {
    assert(addresses != nullptr);
    if (x64_addresses_full(addresses)) { x64_addresses_grow(addresses); }

    u64 index                = addresses->count++;
    addresses->buffer[index] = address;
    return index;
}

x64_Address *x64_addresses_at(x64_Addresses *addresses, u64 index) {
    assert(addresses != nullptr);
    assert(index < addresses->count);
    return addresses->buffer + index;
}
