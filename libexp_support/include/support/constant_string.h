// Copyright 2025 cade-weinberg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef EXP_SUPPORT_CONSTANT_STRING_H
#define EXP_SUPPORT_CONSTANT_STRING_H

#include "support/scalar.h"
#include "support/string_view.h"

typedef struct ConstantString {
    u64        length;
    char const data[];
} ConstantString;

ConstantString *constant_string_create(StringView view);
void            constant_string_destroy(ConstantString *restrict str);

bool constant_string_equal(ConstantString const *restrict string,
                           StringView view);

StringView constant_string_to_view(ConstantString const *restrict str);

#endif // !EXP_SUPPORT_CONSTANT_STRING_H
