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

#include "adt/graph.h"
#include "utility/string_view.h"

typedef struct CFGPair {
  StringView name;
  u64 vertex;
} CFGPair;

typedef struct CFGNameMap {
  u64 size;
  u64 capacity;
  CFGPair *map;
} CFGNameMap;

typedef struct CFGVertexMap {
  u64 size;
  u64 capacity;
  CFGPair *map;
} CFGVertexMap;

typedef struct CFGBimap {
  CFGNameMap names;
  CFGVertexMap vertices;
} CFGBimap;

typedef struct ControlFlowGraph {
  Graph graph;
  CFGBimap bimap;
} ControlFlowGraph;

ControlFlowGraph control_flow_graph_create();
void control_flow_graph_destroy(ControlFlowGraph *restrict cfg);

void control_flow_graph_add_function(ControlFlowGraph *restrict cfg,
                                     StringView name);
void control_flow_graph_add_call(ControlFlowGraph *restrict cfg,
                                 StringView source, StringView target);

typedef struct NameList {
  u64 size;
  u64 capacity;
  StringView *list;
} NameList;

void name_list_destroy(NameList *restrict names);

NameList control_flow_graph_function_fanout(ControlFlowGraph *restrict cfg,
                                            StringView name);
NameList control_flow_graph_function_fanin(ControlFlowGraph *restrict cfg,
                                           StringView name);

#endif // !EXP_ENV_CONTROL_FLOW_GRAPH_H