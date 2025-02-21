/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file test/resource_tests/test_resources.c
 */

#include <ctype.h>
#include <string.h>

#include "test_resources.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/panic.h"

#ifndef EXP_HOST_SYSTEM_LINUX
    #error "unsupported host system"
#endif

#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void test_resources_gather(TestResources *test_resources,
                           char const *directory);

void test_resources_initialize(TestResources *test_resources) {
    EXP_ASSERT(test_resources != nullptr);
    test_resources->count    = 0;
    test_resources->capacity = 0;
    test_resources->buffer   = nullptr;
    test_resources_gather(test_resources, EXP_TEST_RESOURCES_DIR);
}

void test_resources_terminate(TestResources *test_resources) {
    EXP_ASSERT(test_resources != nullptr);
    for (u64 index = 0; index < test_resources->count; ++index) {
        string_terminate(test_resources->buffer + index);
    }
    test_resources->count    = 0;
    test_resources->capacity = 0;
    deallocate(test_resources->buffer);
    test_resources->buffer = nullptr;
}

static bool test_resources_full(TestResources *test_resources) {
    EXP_ASSERT(test_resources != nullptr);
    return (test_resources->count + 1) >= test_resources->capacity;
}

static void test_resources_grow(TestResources *test_resources) {
    EXP_ASSERT(test_resources != nullptr);
    Growth64 g = array_growth_u64(test_resources->capacity, sizeof(String));
    test_resources->buffer   = reallocate(test_resources->buffer, g.alloc_size);
    test_resources->capacity = g.new_capacity;
}

void test_resources_append(TestResources *test_resources,
                           String const *resource) {
    EXP_ASSERT(test_resources != nullptr);
    EXP_ASSERT(resource != nullptr);
    EXP_ASSERT(!string_empty(resource));

    if (test_resources_full(test_resources)) {
        test_resources_grow(test_resources);
    }

    String *string = test_resources->buffer + test_resources->count++;
    string_initialize(string);
    string_assign(string, string_to_view(resource));
}

bool validate_test_resource_name(char const *name);

void test_resources_gather(TestResources *test_resources,
                           char const *directory) {
    EXP_ASSERT(test_resources != nullptr);
    EXP_ASSERT(directory != nullptr);

    DIR *resource_directory = opendir(directory);
    if (resource_directory == nullptr) { PANIC_ERRNO("opendir"); }

    // iterate through each entry in the directory
    struct dirent *directory_entry = nullptr;
    while ((directory_entry = readdir(resource_directory)) != nullptr) {
        char const *name = directory_entry->d_name;
        // hidden file/directory, ignore it
        if (*name == '.') { continue; }

        String full_path;
        string_initialize(&full_path);
        string_append(&full_path, string_view_from_cstring(directory));
        string_append(&full_path, SV("/"));
        string_append(&full_path, string_view_from_cstring(name));

        struct stat entry_stat;
        if (stat(string_to_cstring(&full_path), &entry_stat) < 0) {
            String buffer;
            string_initialize(&buffer);
            string_append(&buffer, string_view_from_cstring(name));
            string_append(&buffer, SV("\n"));
            file_write(string_to_view(&buffer), program_error);
            string_terminate(&buffer);
            PANIC_ERRNO("stat");
        }

        // regular file, treat it as a test resource
        if (S_ISREG(entry_stat.st_mode)) {
            if (!validate_test_resource_name(name)) { continue; }
            test_resources_append(test_resources, &full_path);
        }

        // subdirectory, look for more test resources
        if (S_ISDIR(entry_stat.st_mode)) {
            test_resources_gather(test_resources,
                                  string_to_cstring(&full_path));
        }

        string_terminate(&full_path);
    }

    if (closedir(resource_directory) < 0) { PANIC_ERRNO("closedir"); }
}

bool validate_test_resource_name(char const *name) {
    // check that the name contains the exit code
    u64 cursor = 0;
    if (name[cursor] == '.') { ++cursor; }

    while (true) {
        if (name[cursor] == '.') break;
        if (name[cursor] == '\0') return false;
        ++cursor;
    }
    ++cursor;

    if (!isdigit(name[cursor])) { return false; }
    while (isdigit(name[cursor])) {
        if (name[cursor] == '\0') { return false; }
        ++cursor;
    }

    // check that the name ends in .exp
    if (name[cursor] != '.') { return false; }
    ++cursor;

    if (strcmp(name + cursor, "exp") != 0) { return false; }
    cursor += 3;
    return name[cursor] == '\0';
}
