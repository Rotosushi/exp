
#ifndef EXP_UTILITY_DYNAMIC_ARRAY_H
#define EXP_UTILITY_DYNAMIC_ARRAY_H
#include <assert.h>
#include <stdbool.h>

#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/int_types.h"

#define EXP_DYNAMIC_ARRAY_DECLARATION(                                         \
    DynamicArrayName, DynamicArrayType, DynamicArrayElementType)               \
    typedef struct {                                                           \
        u64 count;                                                             \
        u64 capacity;                                                          \
        DynamicArrayElementType *array;                                        \
    } type_DynamicArrayName;

#define EXP_DYNAMIC_ARRAY_FUNCTION_DECLARATIONS(                               \
    DynamicArrayName, DynamicArrayType, DynamicArrayElementType)               \
    void DynamicArrayName##_initialize(DynamicArrayType *DynamicArrayName);    \
    void DynamicArrayName##_terminate(DynamicArrayType *DynamicArrayName);     \
    void DynamicArrayName##_append(DynamicArrayType *DynamicArrayName,         \
                                   DynamicArrayElementType element);           \
    DynamicArrayElementType *DynamicArrayName##_at(                            \
        DynamicArrayType *DynamicArrayName, u64 index);

#define EXP_DYNAMIC_ARRAY_FUNCTION_DEFINITIONS(                                      \
    DynamicArrayName, DynamicArrayType, DynamicArrayElementType)                     \
    void DynamicArrayName##_initialize(DynamicArrayType *DynamicArrayName) {         \
        assert(DynamicArrayName != nullptr);                                         \
        DynamicArrayName->count    = 0;                                              \
        DynamicArrayName->capacity = 0;                                              \
        DynamicArrayName->array    = nullptr;                                        \
    }                                                                                \
                                                                                     \
    void DynamicArrayName##_terminate(DynamicArrayType *DynamicArrayName) {          \
        assert(DynamicArrayName != nullptr);                                         \
        DynamicArrayName->count    = 0;                                              \
        DynamicArrayName->capacity = 0;                                              \
        deallocate(DynamicArrayName->array);                                         \
    }                                                                                \
                                                                                     \
    bool DynamicArrayName##_full(DynamicArrayType *DynamicArrayName) {               \
        assert(DynamicArrayName != nullptr);                                         \
        return (DynamicArrayName->count + 1) >= name->capacity;                      \
    }                                                                                \
                                                                                     \
    bool DynamicArrayName##_index_in_bounds(                                         \
        DynamicArrayType *DynamicArrayName, u64 index) {                             \
        assert(DynamicArrayName != nullptr);                                         \
        return index < DynamicArrayName->count;                                      \
    }                                                                                \
                                                                                     \
    void DynamicArrayName##_grow(DynamicArrayType *DynamicArrayName) {               \
        assert(DynamicArrayName != nullptr);                                         \
        Growth g                = array_growth_u64(DynamicArrayName->capacity,       \
                                    sizeof(DynamicArrayElementType)); \
        DynamicArrayName->array = reallocate(name->array, g.alloc_size);             \
        DynamicArrayName->capacity = g.new_capacity;                                 \
    }                                                                                \
                                                                                     \
    void DynamicArrayName##_append(DynamicArrayType *DynamicArrayName,               \
                                   DynamicArrayElementType element) {                \
        assert(DynamicArrayName != nullptr);                                         \
        if (DynamicArrayName##_full(name)) { DynamicArrayName##_grow(name); }        \
        DynamicArrayName->array[name->count++] = element;                            \
    }

#endif // EXP_UTILITY_DYNAMIC_ARRAY_H
