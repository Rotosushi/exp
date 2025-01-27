
#include <assert.h>

#include "imr/local_allocator.h"
#include "intrinsics/size_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/unreachable.h"

static void locals_initialize(Locals *locals) {
    assert(locals != nullptr);
    locals->count    = 0;
    locals->capacity = 0;
    locals->buffer   = nullptr;
}

static void locals_terminate(Locals *locals) {
    assert(locals != nullptr);
    deallocate(locals->buffer);
    locals->count    = 0;
    locals->capacity = 0;
    locals->buffer   = nullptr;
}

static bool locals_full(Locals *locals) {
    assert(locals != nullptr);
    return (locals->count + 1) >= locals->capacity;
}

static void locals_grow(Locals *locals) {
    assert(locals != nullptr);
    Growth32 g       = array_growth_u32(locals->capacity, sizeof(Local));
    locals->buffer   = reallocate(locals->buffer, g.alloc_size);
    locals->capacity = g.new_capacity;
}

static u32 locals_allocate(Locals *locals) {
    assert(locals != nullptr);

    if (locals_full(locals)) { locals_grow(locals); }

    u32 index    = locals->count++;
    Local *local = locals->buffer + index;
    local_initialize(local);
    return index;
}

static Local *locals_at(Locals *locals, u32 index) {
    assert(locals != nullptr);
    assert(index < locals->count);
    return locals->buffer + index;
}

/*
static Local *locals_at_name(Locals *locals, StringView name) {
    assert(locals != nullptr);
    assert(!string_view_empty(name));
    for (u64 index = 0; index < locals->count; ++index) {
        Local *cursor = locals->buffer[index];
        if (string_view_equality(cursor->label, name)) { return cursor; }
    }
    return nullptr;
}
*/

void local_allocator_initialize(LocalAllocator *allocator) {
    assert(allocator != nullptr);
    locals_initialize(&allocator->locals);
}

void local_allocator_terminate(LocalAllocator *allocator) {
    assert(allocator != nullptr);
    locals_terminate(&allocator->locals);
}

u32 local_allocator_declare_ssa(LocalAllocator *allocator) {
    assert(allocator != nullptr);
    return locals_allocate(&allocator->locals);
}

Local *local_allocator_at(LocalAllocator *allocator, u32 ssa) {
    assert(allocator != nullptr);
    return locals_at(&allocator->locals, ssa);
}

/*
Local *local_allocator_at_name(LocalAllocator *allocator, StringView name) {
    assert(allocator != nullptr);
    return locals_at_name(&allocator->locals, name);
}
*/

static void deallocate_register(LocalAllocator *allocator, u32 position) {
    assert(allocator != nullptr);
    assert(position < bitset_length());
    assert(bitset_check_bit(&allocator->registers, (u8)position));
    bitset_clear_bit(&allocator->registers, (u8)position);
}

static void deallocate_local(LocalAllocator *allocator, Local *local) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    switch (local->location.kind) {
    case LOCATION_UNINITIALIZED: EXP_UNREACHABLE();
    case LOCATION_GENERAL_PURPOSE_REGISTER:
        deallocate_register(allocator,
                            local->location.data.general_purpose_register);
        local->location = location_create();
        break;
    // #NOTE: we are not handling deallocating stack slots just yet.
    //  as it isn't strictly necessary for a unoptimized build.
    case LOCATION_STACK_SLOT: return;
    default:                  EXP_UNREACHABLE();
    }
}

static void deallocate_expired_locals(LocalAllocator *allocator,
                                      u32 block_index) {
    assert(allocator != nullptr);
    Locals *locals = &allocator->locals;
    for (u32 index = 0; index < locals->count; ++index) {
        Local *local = locals->buffer + index;
        if (local->lifetime.last_use < block_index) {
            deallocate_local(allocator, local);
        }
    }
}

static bool first_available_register(LocalAllocator *allocator, u8 *register_) {
    assert(allocator != nullptr);
    assert(register_ != nullptr);

    u32 bits = ~allocator->registers.bits;
    if (bits == 0) return false;
    int index = __builtin_ctz(bits);
    assert(index < bitset_length());
    *register_ = (u8)index;
    return true;
    // for (u8 index = 0; index < bitset_length(); ++index) {
    //    if (bitset_check_bit(&allocator->registers, index)) continue;
    //     *register_ = index;
    //     return true;
    // }
    // return false;
}

static bool register_allocate(LocalAllocator *allocator, Local *local) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    assert(local->type != nullptr);
    if (size_of(local->type) > 8) { return false; }
    u8 register_;
    if (!first_available_register(allocator, &register_)) { return false; }
    bitset_set_bit(&allocator->registers, register_);
    local_update_location(local, location_register(register_));
    return true;
}

static void stack_allocate(LocalAllocator *allocator, Local *local) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    assert(local->type != nullptr);
    local_update_location(local, location_stack_slot(allocator->stack_slots++));
}

// #NOTE: since we aren't tracking actual registers or stack slots
//  we don't have any ability to conform to the ABI of the platform
//  here. Essentially we are allocating abstract registers and stack
//  slots.
/*
void local_allocator_allocate_result(LocalAllocator *allocator, Local *local) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    assert(local->type != nullptr);
    assert(false); // #TODO:
}

void local_allocator_allocate_formal_argument(LocalAllocator *allocator,
                                              Local *local,
                                              u8 argument_index) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    assert(local->type != nullptr);
    assert(false); // #TODO:
}

void local_allocator_allocate_actual_argument(LocalAllocator *allocator,
                                              Local *local,
                                              u8 argument_index,
                                              u32 block_index) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    assert(local->type != nullptr);
    assert(false); // #TODO:
}
*/

void local_allocator_allocate_local(LocalAllocator *allocator,
                                    Local *local,
                                    u32 block_index) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    assert(local->type != nullptr);

    deallocate_expired_locals(allocator, block_index);

    if (register_allocate(allocator, local)) { return; }

    stack_allocate(allocator, local);
}
