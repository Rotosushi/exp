
#include "test_resource.h"
#include "test_resources.h"
#include "utility/io.h"
#include "utility/result.h"

i32 main() {
    i32 result = EXP_SUCCESS;
    TestResources test_resources;
    test_resources_initialize(&test_resources);

    for (u64 index = 0; index < test_resources.count; ++index) {
        String *resource = test_resources.buffer + index;
        String buffer;
        string_initialize(&buffer);
        string_append(&buffer, SV("\ntesting resource: "));
        string_append(&buffer, string_to_view(resource));
        file_write(string_to_view(&buffer), program_error);
        string_terminate(&buffer);
        result |= test_resource(string_to_view(resource));
    }

    test_resources_terminate(&test_resources);
    return result;
}
