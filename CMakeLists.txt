cmake_minimum_required(VERSION 3.20)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(EXP_HOST_OS_LINUX 1)
else()
  set(EXP_HOST_OS_LINUX 0)
  message(FATAL_ERROR "Unsupported Host OS")
endif()

set(C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)

project(EXP LANGUAGES C VERSION 0.0.1 DESCRIPTION "A hobby programming language.")

set(EXP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(EXP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/source")
set(EXP_TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(EXP_BUILD_DIR "${PROJECT_BINARY_DIR}")

set(WARNINGS -Wall -Wdeprecated -Wextra -Wpedantic -Wconversion -Werror)


execute_process(
  COMMAND git rev-parse HEAD 
  OUTPUT_VARIABLE EXP_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ($<CONFIG:Debug> OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(EXP_DEBUG 1)
else()
  set(EXP_DEBUG 0)
endif()

string(TIMESTAMP EXP_BUILD_TIME "%Y-%m-%d %H:%M:%S")

set(EXP_VERSION_STRING "")
string(APPEND EXP_VERSION_STRING 
"exp version ("
${EXP_VERSION_MAJOR}
"."
${EXP_VERSION_MINOR}
"."
${EXP_VERSION_PATCH}
") built at ("
${EXP_BUILD_TIME}
") git revision ("
${EXP_GIT_REVISION}
")"
)

configure_file(
  ${EXP_INCLUDE_DIR}/utility/config.h.in 
  ${EXP_INCLUDE_DIR}/utility/config.h
)

add_subdirectory(${EXP_SOURCE_DIR})

include(CTest)
add_subdirectory(${EXP_TEST_DIR})


