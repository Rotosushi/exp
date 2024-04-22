// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_ENV_CONTROL_FLOW_GRAPH_H
#define EXP_ENV_CONTROL_FLOW_GRAPH_H

#include "utility/graph.h"
#include "utility/string_view.h"

typedef struct NamePair {
  StringView name;
  u64 tag;
} NamePair;

typedef struct NameMap {
  u64 size;
  u64 capacity;
  NamePair *map;
} NameMap;

typedef struct ControlFlowGraph {
  Graph graph;
  NameMap names;
} ControlFlowGraph;

ControlFlowGraph control_flow_graph_create();
void control_flow_graph_destroy(ControlFlowGraph *restrict cfg);

void control_flow_graph_add_function(ControlFlowGraph *restrict cfg,
                                     StringView name);
void control_flow_graph_add_call(ControlFlowGraph *restrict cfg,
                                 StringView source, StringView target);

VertexList control_flow_graph_function_fanout(ControlFlowGraph *restrict cfg,
                                              StringView name);
VertexList control_flow_graph_function_fanin(ControlFlowGraph *restrict cfg,
                                             StringView name);

#endif // !EXP_ENV_CONTROL_FLOW_GRAPH_H