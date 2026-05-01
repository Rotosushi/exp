

#
# Copyright (c) 2025 Alan de Freitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/url
#

# This file defines a script that can be used to create another
# CMake script that will discover tests from a Boost.URL test suite executable
# and write them to a CTest script file.

# Define a custom message function that prints the message only if some
# special debug variable is set from the command line when calling the script.
# This is useful to avoid cluttering the output with debug messages but allowing
# us to debug this script since the CMake debugger cannot step in here.
function(debugMessage)
    if (TEST_SUITE_SCRIPT_DEBUG)
        message(STATUS "${ARGV0}")
    endif ()
endfunction()

# Appends a formatted CMake command to the FULL_SCRIPT variable in the parent scope.
#
# Arguments:
#   NAME         - The CMake command name (e.g., add_test, set).
#   ...          - The arguments to the command. Each argument is checked for special
#                  characters; if present, it is wrapped as a CMake bracket argument [==[...]==].
#
# Details:
#   - Arguments are processed to ensure correct escaping for script generation.
#   - The resulting command line is appended to the FULL_SCRIPT variable in the parent scope,
#     followed by a newline.
#
# Example:
#   pushCommand(add_test my_test /path/to/exe "test name with spaces")
#   # Appends: add_test(my_test /path/to/exe [==[test name with spaces]==])
function(pushCommand NAME)
    set(PUSH_COMMAND_ARGS "")
    math(EXPR PUSH_COMMAND_LAST_ARG_INDEX ${ARGC}-1)
    foreach (PUSH_COMMAND_ARG_IDX RANGE 1 ${PUSH_COMMAND_LAST_ARG_INDEX})
        set(PUSH_COMMAND_ARG "${ARGV${PUSH_COMMAND_ARG_IDX}}")
        if (PUSH_COMMAND_ARG MATCHES "[^-./:a-zA-Z0-9_]")
            set(PUSH_COMMAND_ARGS "${PUSH_COMMAND_ARGS} [==[${PUSH_COMMAND_ARG}]==]") # form a bracket_argument
        else ()
            set(PUSH_COMMAND_ARGS "${PUSH_COMMAND_ARGS} ${PUSH_COMMAND_ARG}")
        endif ()
    endforeach ()
    set(FULL_SCRIPT "${FULL_SCRIPT}${NAME}(${PUSH_COMMAND_ARGS})\n" PARENT_SCOPE)
    debugMessage("Adding command to script: ${NAME}(${PUSH_COMMAND_ARGS})")
endfunction()

# Function to write CTest script for discovering tests from a Boost.URL test suite executable
# This function discovers tests from an executable target that uses the Boost.URL test suite
# and creates individual CTest targets for each test.
# The tests are defined in an external script that is later included by CMake.
function(discoverTestsImpl)
    debugMessage("Parsing test parameters...")
    cmake_parse_arguments(
            # prefix
            "TEST_SUITE"
            # options
            ""
            # single-value arguments
            "TEST_TARGET;TEST_EXECUTABLE;TEST_WORKING_DIR;TEST_PREFIX;TEST_SPEC;TEST_SUFFIX;CTEST_FILE"
            # multi-value arguments
            "TEST_EXTRA_ARGS;TEST_PROPERTIES"
            ${ARGN}
    )

    # Variables for arguments
    if (NOT EXISTS "${TEST_SUITE_TEST_EXECUTABLE}")
        debugMessage(FATAL_ERROR "Specified test executable '${TEST_SUITE_TEST_EXECUTABLE}' does not exist")
    endif ()

    # Run the test executable to get a list of available tests
    debugMessage("Running test executable to discover tests...")
    debugMessage("${TEST_SUITE_TEST_EXECUTABLE} ${TEST_SUITE_TEST_SPEC} --list_content")
    execute_process(
            COMMAND "${TEST_SUITE_TEST_EXECUTABLE}" ${TEST_SUITE_TEST_SPEC} --list_content
            OUTPUT_VARIABLE TEST_SUITE_LIST_TESTS_OUTPUT
            ERROR_VARIABLE TEST_SUITE_LIST_TESTS_OUTPUT
            RESULT_VARIABLE TEST_SUITE_RESULT
            WORKING_DIRECTORY "${TEST_SUITE_TEST_WORKING_DIR}"
    )
    if (NOT ${TEST_SUITE_RESULT} EQUAL 0)
        debugMessage(FATAL_ERROR
                "Error listing tests from executable '${TEST_SUITE_TEST_EXECUTABLE}':\n"
                "  Result: ${TEST_SUITE_RESULT}\n"
                "  Output: ${TEST_SUITE_LIST_TESTS_OUTPUT}\n"
        )
    endif ()

    # Split lines into a list
    # Normalize line endings to \n
    string(REPLACE "\r\n" "\n" TEST_SUITE_LIST_TESTS_OUTPUT "${TEST_SUITE_LIST_TESTS_OUTPUT}")
    string(REPLACE "\r" "\n" TEST_SUITE_LIST_TESTS_OUTPUT "${TEST_SUITE_LIST_TESTS_OUTPUT}")
    string(REPLACE "*" "" TEST_SUITE_LIST_TESTS_OUTPUT "${TEST_SUITE_LIST_TESTS_OUTPUT}")
    separate_arguments(TEST_SUITE_LIST UNIX_COMMAND "${TEST_SUITE_LIST_TESTS_OUTPUT}")
    debugMessage("Discovered tests: ${TEST_SUITE_LIST}")

    # Exit early if no tests are detected
    list(LENGTH TEST_SUITE_LIST TEST_SUITE_NUMBER_OF_TESTS)
    if (TEST_SUITE_NUMBER_OF_TESTS EQUAL 0)
        return()
    endif ()
    debugMessage("Number of discovered tests: ${TEST_SUITE_NUMBER_OF_TESTS}")

    set(FULL_SCRIPT "# This file is automatically generated by CMake to define tests discovered in the given TARGET test suite.\n")
    set(TEST_SUITE_CMAKE_TESTS)
    foreach (TEST_SUITE_PLAIN_NAME IN LISTS TEST_SUITE_LIST)
        # Escape test names
        set(TEST_SUITE_ESCAPED_NAME "${TEST_SUITE_PLAIN_NAME}")

        foreach (char \\ , [ ] ;)
            string(REPLACE ${char} "\\${char}" TEST_SUITE_ESCAPED_NAME "${TEST_SUITE_ESCAPED_NAME}")
        endforeach ()

        # Add test to file
        set(CTEST_TEST_NAME "${TEST_SUITE_TEST_PREFIX}${TEST_SUITE_PLAIN_NAME}${TEST_SUITE_TEST_SUFFIX}")
        pushCommand(add_test
                "${CTEST_TEST_NAME}"
                "${TEST_SUITE_TEST_EXECUTABLE}"
                "${TEST_SUITE_ESCAPED_NAME}"
                ${TEST_SUITE_TEST_EXTRA_ARGS}
        )

        # Append to the list of tests
        list(APPEND TEST_SUITE_CMAKE_TESTS "${CTEST_TEST_NAME}")
    endforeach ()

    # Debug message showing the file we're about to write
    debugMessage("Writing test suite script to: ${TEST_SUITE_CTEST_FILE}")

    file(WRITE "${TEST_SUITE_CTEST_FILE}" "${FULL_SCRIPT}")
endfunction()

if (CMAKE_SCRIPT_MODE_FILE)
    debugMessage("Discovering tests for ${TEST_TARGET}")
    debugMessage("Test executable: ${TEST_EXECUTABLE}")
    debugMessage("Test target: ${TEST_TARGET}")
    if (TEST_WORKING_DIR)
        debugMessage("Working directory: ${TEST_WORKING_DIR}")
    endif ()
    if (TEST_SPEC)
        debugMessage("Test spec: ${TEST_SPEC}")
    endif ()
    if (TEST_EXTRA_ARGS)
        debugMessage("Extra arguments: ${TEST_EXTRA_ARGS}")
    endif ()
    if (TEST_PROPERTIES)
        debugMessage("Test properties: ${TEST_PROPERTIES}")
    endif ()
    if (TEST_PREFIX)
        debugMessage("Test prefix: ${TEST_PREFIX}")
    endif ()
    if (TEST_SUFFIX)
        debugMessage("Test suffix: ${TEST_SUFFIX}")
    endif ()
    debugMessage("CTest file: ${CTEST_FILE}")
    # Call the function to write the test file
    discoverTestsImpl(
            TEST_TARGET ${TEST_TARGET}
            TEST_EXECUTABLE ${TEST_EXECUTABLE}
            TEST_WORKING_DIR ${TEST_WORKING_DIR}
            TEST_SPEC ${TEST_SPEC}
            TEST_EXTRA_ARGS ${TEST_EXTRA_ARGS}
            TEST_PROPERTIES ${TEST_PROPERTIES}
            TEST_PREFIX ${TEST_PREFIX}
            TEST_SUFFIX ${TEST_SUFFIX}
            CTEST_FILE ${CTEST_FILE}
    )
endif ()

