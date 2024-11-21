

#ifndef EXP_TEST_END_TO_END_TEST_RESOURCES_H
#define EXP_TEST_END_TO_END_TEST_RESOURCES_H

#include "adt/string.h"

typedef struct TestResources {
    u64 count;
    u64 capacity;
    String *buffer;
} TestResources;

void test_resources_initialize(TestResources *test_resources);
void test_resources_terminate(TestResources *test_resources);

#endif // EXP_TEST_END_TO_END_TEST_RESOURCES_H
