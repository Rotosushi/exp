// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file test/resource_tests/test_resources.h
 */

#ifndef EXP_TEST_RESOURCE_TESTS_TEST_RESOURCES_H
#define EXP_TEST_RESOURCE_TESTS_TEST_RESOURCES_H

#include "utility/string.h"

typedef struct TestResources {
    u64 count;
    u64 capacity;
    String *buffer;
} TestResources;

void test_resources_initialize(TestResources *test_resources);
void test_resources_terminate(TestResources *test_resources);

#endif // EXP_TEST_RESOURCE_TESTS_TEST_RESOURCES_H
