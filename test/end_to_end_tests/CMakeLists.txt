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

add_executable(test_all_resources 
  ${EXP_TEST_DIR}/end_to_end_tests/main.c
)
target_include_directories(test_all_resources PUBLIC
  ${EXP_LIBEXP_UTILITY_INCLUDE_DIR}
  ${EXP_INCLUDE_DIR}
  ${EXP_TEST_DIR}/libexp_test
)
target_compile_options(test_all_resources PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(test_all_resources PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(test_all_resources PUBLIC exp_test)

add_test(NAME "test: compile all resources and check return value of emitted program" COMMAND test_all_resources)

#create_test_sourcelist(Tests exp_e2e_tests.c
#  addition.c
#  call.c
#  comment.c
#  division.c
#  modulus.c
#  multiply.c
#  pemdas.c
#  return_.c
#  subtraction.c
#  tuple.c
#)

#add_executable (exp_e2e_tests ${Tests})
#target_include_directories(exp_e2e_tests PUBLIC ${EXP_INCLUDE_DIR})
#target_compile_options(exp_e2e_tests PUBLIC ${EXP_COMPILE_OPTIONS})
#target_link_options(exp_e2e_tests PUBLIC ${EXP_LINK_OPTIONS})
#target_link_libraries(exp_e2e_tests PUBLIC exp_common exp_e2e m)


#set (TestsToRun ${Tests})
#remove (TestsToRun exp_e2e_tests.c)

#foreach (test ${TestsToRun})
#  get_filename_component (TestName ${test} NAME_WE)
#  add_test (NAME ${TestName} COMMAND exp_e2e_tests ${TestName})
#endforeach()
