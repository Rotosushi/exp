
// #include <assert.h>
// #include <string.h>

#include "utility/constant_string.h"
#include "utility/alloc.h"
#include "utility/assert.h"
#include "utility/memory.h"

ConstantString *constant_string_allocate(StringView contents) {
    ConstantString *string =
        callocate(1, sizeof(ConstantString) + contents.length + 1);
    EXP_ASSERT(string != nullptr);
    string->length = contents.length;
    memory_copy(string->buffer, string->length, contents.ptr, contents.length);
    return string;
}

void constant_string_deallocate(ConstantString *string) {
    EXP_ASSERT(string != nullptr);
    deallocate(string);
}

StringView constant_string_to_view(ConstantString *string) {
    EXP_ASSERT(string != nullptr);
    return string_view_from_str(string->buffer, string->length);
}

bool constant_string_equality(ConstantString *string, StringView view) {
    EXP_ASSERT(string != nullptr);
    if (string->length != view.length) return false;
    return memory_compare(
               string->buffer, string->length, view.ptr, view.length) == 0;
}
