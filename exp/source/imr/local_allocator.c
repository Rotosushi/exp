
#include <assert.h>

#include "imr/local_allocator.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

static void locals_initialize(Locals *locals) {
    assert(locals != nullptr);
    locals->count    = 0;
    locals->capacity = 0;
    locals->buffer   = nullptr;
}

static void locals_terminate(Locals *locals) {
    assert(locals != nullptr);
    for (u32 i = 0; i < locals->count; ++i) {
        Local **cursor = locals->buffer + i;
        if (*cursor == nullptr) { continue; }
        deallocate(*cursor);
        *cursor = nullptr;
    }

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
    Growth32 g       = array_growth_u32(locals->capacity, sizeof(Local *));
    locals->buffer   = reallocate(locals->buffer, g.alloc_size);
    locals->capacity = g.new_capacity;
}

static u32 locals_allocate(Locals *locals) {
    assert(locals != nullptr);

    if (locals_full(locals)) { locals_grow(locals); }

    u32 index     = locals->count++;
    Local **local = locals->buffer + index;
    *local        = allocate(sizeof(Local));
    local_initialize(*local);
    return index;
}

static Local *locals_at(Locals *locals, u32 index) {
    assert(locals != nullptr);
    assert(index < locals->count);
    return locals->buffer[index];
}

static Local *locals_at_name(Locals *locals, StringView name) {
    assert(locals != nullptr);
    assert(!string_view_empty(name));
    for (u32 index = 0; index < locals->count; ++index) {
        Local *cursor = locals->buffer[index];
        if (string_view_equality(cursor->label, name)) { return cursor; }
    }
    return nullptr;
}

void local_allocator_initialize(LocalAllocator *allocator) {
    assert(allocator != nullptr);
    locals_initialize(&allocator->locals);
}

void local_allocator_terminate(LocalAllocator *allocator) {
    assert(allocator != nullptr);
    locals_terminate(&allocator->locals);
}

u32 local_allocator_allocate(LocalAllocator *allocator) {
    assert(allocator != nullptr);
    return locals_allocate(&allocator->locals);
}

Local *local_allocator_at(LocalAllocator *allocator, u32 ssa) {
    assert(allocator != nullptr);
    return locals_at(&allocator->locals, ssa);
}

Local *local_allocator_at_name(LocalAllocator *allocator, StringView name) {
    assert(allocator != nullptr);
    return locals_at_name(&allocator->locals, name);
}
