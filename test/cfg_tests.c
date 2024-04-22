/**
 * Copyright (C) 2024 Cade Weinberg
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
#include <stdlib.h>

#include "env/control_flow_graph.h"

// static bool list_contains(VertexList *restrict vl, u64 vertex) {
//   for (u64 i = 0; i < vl->count; ++i) {
//     if (vl->list[i] == vertex) {
//       return 1;
//     }
//   }
//   return 0;
// }

int cfg_tests([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  bool failure         = 0;
  ControlFlowGraph cfg = control_flow_graph_create();
  StringView f0        = string_view_from_cstring("f0");
  StringView f1        = string_view_from_cstring("f1");
  StringView f2        = string_view_from_cstring("f2");
  StringView f3        = string_view_from_cstring("f3");

  control_flow_graph_add_function(&cfg, f0);
  control_flow_graph_add_function(&cfg, f1);
  control_flow_graph_add_function(&cfg, f2);
  control_flow_graph_add_function(&cfg, f3);
  /*
  f0 -> f1
  f0 -> f2
  f1 -> f3
  f2 -> f3
  f3 -> NULL
  */
  control_flow_graph_add_call(&cfg, f0, f1);
  control_flow_graph_add_call(&cfg, f0, f2);
  control_flow_graph_add_call(&cfg, f1, f3);
  control_flow_graph_add_call(&cfg, f2, f3);

  VertexList f0_fanout = control_flow_graph_function_fanout(&cfg, f0);
  failure |= (f0_fanout.count != 2);

  control_flow_graph_destroy(&cfg);
  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}