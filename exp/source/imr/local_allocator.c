
// #include <EXP_ASSERT.h>

#include "imr/local_allocator.h"
#include "intrinsics/size_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

/*
static Local *locals_at_name(Locals *locals, StringView name) {
    EXP_ASSERT(locals != nullptr);
    EXP_ASSERT(!string_view_empty(name));
    for (u64 index = 0; index < locals->count; ++index) {
        Local *cursor = locals->buffer[index];
        if (string_view_equality(cursor->label, name)) { return cursor; }
    }
    return nullptr;
}
*/

void local_allocator_initialize(LocalAllocator *allocator) {
    EXP_ASSERT(allocator != nullptr);
    locals_initialize(&allocator->locals);
}

void local_allocator_terminate(LocalAllocator *allocator) {
    EXP_ASSERT(allocator != nullptr);
    locals_terminate(&allocator->locals);
}

u32 local_allocator_declare_ssa(LocalAllocator *allocator) {
    EXP_ASSERT(allocator != nullptr);
    return locals_allocate(&allocator->locals);
}

Local *local_allocator_at(LocalAllocator *allocator, u32 ssa) {
    EXP_ASSERT(allocator != nullptr);
    return locals_at(&allocator->locals, ssa);
}

/*
Local *local_allocator_at_name(LocalAllocator *allocator, StringView name) {
    EXP_ASSERT(allocator != nullptr);
    return locals_at_name(&allocator->locals, name);
}
*/

static void deallocate_register(LocalAllocator *allocator, u32 position) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(position < bitset_length());
    EXP_ASSERT(bitset_check_bit(&allocator->registers, (u8)position));
    bitset_clear_bit(&allocator->registers, (u8)position);
}

static void deallocate_local(LocalAllocator *allocator, Local *local) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(local != nullptr);
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
    EXP_ASSERT(allocator != nullptr);
    Locals *locals = &allocator->locals;
    for (u32 index = 0; index < locals->count; ++index) {
        Local *local = locals->buffer + index;
        if (local->lifetime.last_use < block_index) {
            deallocate_local(allocator, local);
        }
    }
}

static bool first_available_register(LocalAllocator *allocator, u8 *register_) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(register_ != nullptr);

    u32 bits = ~allocator->registers.bits;
    if (bits == 0) return false;
    int index = __builtin_ctz(bits);
    EXP_ASSERT(index < bitset_length());
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
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(local != nullptr);
    EXP_ASSERT(local->type != nullptr);
    if (size_of(local->type) > 8) { return false; }
    u8 register_;
    if (!first_available_register(allocator, &register_)) { return false; }
    bitset_set_bit(&allocator->registers, register_);
    local_update_location(local, location_register(register_));
    return true;
}

static void stack_allocate(LocalAllocator *allocator, Local *local) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(local != nullptr);
    EXP_ASSERT(local->type != nullptr);
    local_update_location(local, location_stack_slot(allocator->stack_slots++));
}

// #NOTE: since we aren't tracking actual registers or stack slots
//  we don't have any ability to conform to the ABI of the platform
//  here. Essentially we are allocating abstract registers and stack
//  slots.
/*
void local_allocator_allocate_result(LocalAllocator *allocator, Local *local) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(local != nullptr);
    EXP_ASSERT(local->type != nullptr);
    EXP_ASSERT(false); // #TODO:
}

void local_allocator_allocate_formal_argument(LocalAllocator *allocator,
                                              Local *local,
                                              u8 argument_index) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(local != nullptr);
    EXP_ASSERT(local->type != nullptr);
    EXP_ASSERT(false); // #TODO:
}

void local_allocator_allocate_actual_argument(LocalAllocator *allocator,
                                              Local *local,
                                              u8 argument_index,
                                              u32 block_index) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(local != nullptr);
    EXP_ASSERT(local->type != nullptr);
    EXP_ASSERT(false); // #TODO:
}
*/

void local_allocator_allocate_local(LocalAllocator *allocator,
                                    Local *local,
                                    u32 block_index) {
    EXP_ASSERT(allocator != nullptr);
    EXP_ASSERT(local != nullptr);
    EXP_ASSERT(local->type != nullptr);

    deallocate_expired_locals(allocator, block_index);

    if (register_allocate(allocator, local)) { return; }

    stack_allocate(allocator, local);
}
