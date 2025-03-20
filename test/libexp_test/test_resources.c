
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/config.h"
#include "support/io.h"
#include "support/log.h"
#include "support/panic.h"
#include "test_resources.h"

void test_resources_gather(TestResources *test_resources,
                           char const *directory);

void test_resources_initialize(TestResources *test_resources) {
    assert(test_resources != nullptr);
    test_resources->count    = 0;
    test_resources->capacity = 0;
    test_resources->buffer   = nullptr;
    test_resources_gather(test_resources, EXP_TEST_RESOURCES_DIR);
}

void test_resources_terminate(TestResources *test_resources) {
    assert(test_resources != nullptr);
    for (u64 index = 0; index < test_resources->count; ++index) {
        string_destroy(test_resources->buffer + index);
    }
    test_resources->count    = 0;
    test_resources->capacity = 0;
    deallocate(test_resources->buffer);
    test_resources->buffer = nullptr;
}

static bool test_resources_full(TestResources *test_resources) {
    assert(test_resources != nullptr);
    return (test_resources->count + 1) >= test_resources->capacity;
}

static void test_resources_grow(TestResources *test_resources) {
    assert(test_resources != nullptr);
    Growth_u64 g = array_growth_u64(test_resources->capacity, sizeof(String));
    test_resources->buffer   = reallocate(test_resources->buffer, g.alloc_size);
    test_resources->capacity = g.new_capacity;
}

void test_resources_append(TestResources *test_resources,
                           String const *resource) {
    assert(test_resources != nullptr);
    assert(resource != nullptr);
    assert(!string_empty(resource));
    if (test_resources_full(test_resources)) {
        test_resources_grow(test_resources);
    }

    String *string = test_resources->buffer + test_resources->count++;
    *string        = string_create();
    string_assign(string, string_to_view(resource));
}

#if defined(EXP_HOST_SYSTEM_LINUX)
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void test_resources_gather(TestResources *test_resources,
                           char const *directory) {
    assert(test_resources != nullptr);
    assert(directory != nullptr);

    DIR *resource_directory = opendir(directory);
    if (resource_directory == nullptr) {
        exp_log(LOG_ERROR, NULL, 0, SV("opendir failed"), stderr);
        exp_log(LOG_STATUS, NULL, 0, SV("directory: "), stderr);
        exp_log(
            LOG_STATUS, NULL, 0, string_view_from_cstring(directory), stderr);
        exp_log(LOG_STATUS, NULL, 0, SV("errno:"), stderr);
        exp_log(LOG_STATUS,
                NULL,
                0,
                string_view_from_cstring(strerror(errno)),
                stderr);
        return;
    }

    // iterate through each entry in the directory
    struct dirent *directory_entry = nullptr;
    while ((directory_entry = readdir(resource_directory)) != nullptr) {
        char const *name = directory_entry->d_name;
        if (*name == '.') { continue; }

        String full_path = string_create();
        string_append(&full_path, string_view_from_cstring(directory));
        string_append(&full_path, SV("/"));
        string_append(&full_path, string_view_from_cstring(name));

        struct stat entry_stat;
        if (stat(string_to_cstring(&full_path), &entry_stat) < 0) {
            file_write(string_view_from_cstring(name), stderr);
            file_write(SV("\n"), stderr);
            PANIC_ERRNO("stat");
        }

        // regular file, treat it as a test resource
        if (S_ISREG(entry_stat.st_mode)) {
            // skip any files which are not source code (*.exp)
            StringView extension = string_extension(&full_path);
            if (!string_view_equality(extension, SV(".exp"))) {
                string_destroy(&full_path);
                continue;
            }
            test_resources_append(test_resources, &full_path);
        }

        // subdirectory, look for more test resources
        if (S_ISDIR(entry_stat.st_mode)) {
            test_resources_gather(test_resources,
                                  string_to_cstring(&full_path));
        }

        string_destroy(&full_path);
    }

    if (closedir(resource_directory) < 0) { PANIC_ERRNO("closedir"); }
}

#elif defined(EXP_HOST_SYSTEM_WINDOWS)
#error "TODO"
#else
#error "unsupported host system"
#endif
