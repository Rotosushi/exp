
#include <assert.h>
#include <string.h>

#include "utility/alloc.h"
#include "utility/constant_string.h"

ConstantString *constant_string_allocate(StringView contents) {
    ConstantString *string =
        callocate(1, sizeof(ConstantString) + contents.length + 1);
    assert(string != nullptr);
    string->length = contents.length;
    memcpy(string->buffer, contents.ptr, contents.length);
    return string;
}

void constant_string_deallocate(ConstantString *string) {
    assert(string != nullptr);
    deallocate(string);
}

StringView constant_string_to_view(ConstantString *string) {
    assert(string != nullptr);
    return string_view_from_str(string->buffer, string->length);
}

bool constant_string_equality(ConstantString *string, StringView view) {
    assert(string != nullptr);
    if (string->length != view.length) return false;
    return strncmp(string->buffer, view.ptr, view.length) == 0;
}
