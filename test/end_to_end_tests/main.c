
#include <stdlib.h>

#include "test_exp.h"
#include "test_resources.h"

i32 main() {
    i32 result = EXIT_SUCCESS;
    TestResources test_resources;
    test_resources_initialize(&test_resources);

    for (u64 index = 0; index < test_resources.count; ++index) {
        String *resource = test_resources.buffer + index;
        file_write("\ntesting resource: ", stderr);
        file_write(string_to_cstring(resource), stderr);
        result |= test_source(string_to_view(resource));
    }

    test_resources_terminate(&test_resources);
    return result;
}
