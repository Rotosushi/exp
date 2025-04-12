/**
 * Copyright 2025 cade-weinberg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>

#include "support/allocation.h"
#include "support/assert.h"
#include "support/constant_string.h"

ConstantString *constant_string_create(StringView view) {
    ConstantString *str =
        callocate(1, sizeof(ConstantString) + view.length + 1);
    str->length = view.length;
    memcpy((void *)str->data, view.ptr, view.length);
    return str;
}

void constant_string_destroy(ConstantString *restrict str) {
    exp_assert(str != NULL);
    deallocate(str);
}

StringView constant_string_to_view(ConstantString const *restrict str) {
    exp_assert(str != NULL);
    return string_view_from_str(str->data, str->length);
}
