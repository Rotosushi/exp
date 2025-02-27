# Copyright (C) 2024 Cade Weinberg
# 
# This file is part of exp.
# 
# exp is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# exp is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with exp.  If not, see <https://www.gnu.org/licenses/>.
cmake_minimum_required(VERSION 3.20)

set(LIBE2E_SOURCES
  ${EXP_TEST_DIR}/resource_tests/test_resource.c
  ${EXP_TEST_DIR}/resource_tests/test_resources.c
)

add_library(exp_resource_tests ${LIBE2E_SOURCES})
target_include_directories(exp_resource_tests PUBLIC ${EXP_TEST_E2E_DIR})
target_compile_options(exp_resource_tests PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_resource_tests PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp_resource_tests PUBLIC exp_common)

add_executable(test_all_resources 
  ${EXP_TEST_DIR}/resource_tests/main.c
)
target_include_directories(test_all_resources PUBLIC ${EXP_TEST_E2E_DIR})
target_compile_options(test_all_resources PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(test_all_resources PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(test_all_resources PUBLIC exp_resource_tests)

add_test(NAME "test all resources" COMMAND test_all_resources)

