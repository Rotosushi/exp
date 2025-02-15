# Copyright 2025 Cade Weinberg. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
cmake_minimum_required(VERSION 3.20)

set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_CALL_GRAPH YES)
set(DOXYGEN_CALLER_GRAPH YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_RECURSIVE YES)
set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)

doxygen_add_docs(docs
    ${EXP_INCLUDE_DIR}
    ${EXP_SOURCE_DIR}
    ${EXP_DOCS_DIR}
    ${EXP_LIBEXP_RUNTIME_SOURCE_DIR}
    ALL
)
