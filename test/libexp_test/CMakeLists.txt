# Copyright (C) 2025 Cade Weinberg
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

set(LIBEXP_TEST_SOURCES
  ${EXP_TEST_DIR}/libexp_test/test_exp.c
  ${EXP_TEST_DIR}/libexp_test/test_resources.c
)

add_library(exp_test ${LIBEXP_TEST_SOURCES})
target_include_directories(exp_test PRIVATE 
${EXP_INCLUDE_DIR}
${EXP_TEST_DIR}/libexp_test
)
target_compile_options(exp_test PRIVATE ${EXP_COMPILE_OPTIONS})
target_link_options(exp_test PRIVATE ${EXP_LINK_OPTIONS})
target_link_libraries(exp_test PRIVATE exp_utility)

