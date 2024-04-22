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
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "env/control_flow_graph.h"
#include "utility/array_growth.h"
#include "utility/panic.h"
#include "utility/string_hash.h"

#define NAME_MAP_MAX_LOAD 0.75

static NameMap name_map_create() {
  NameMap map;
  map.size     = 0;
  map.capacity = 0;
  map.map      = NULL;
  return map;
}

static void name_map_destroy(NameMap *restrict map) {
  map->size     = 0;
  map->capacity = 0;
  free(map->map);
  map->map = NULL;
}

static NamePair *name_map_find(NamePair *map, u64 capacity, StringView name) {
  u64 index = string_hash(name.ptr, name.length) % capacity;
  while (1) {
    NamePair *pair = map + index;
    if ((pair->name.ptr == NULL) || (string_view_equality(name, pair->name))) {
      return pair;
    }
    index = (index + 1) % capacity;
  }
}

static bool name_map_full(NameMap *restrict map) {
  u64 load_limit = (u64)floor((double)map->capacity * NAME_MAP_MAX_LOAD);
  return (map->size + 1) >= load_limit;
}

static void name_map_grow(NameMap *restrict map) {
  Growth g         = array_growth(map->capacity, sizeof(NamePair));
  NamePair *buffer = calloc(g.new_capacity, sizeof(NamePair));

  if (map->map != NULL) {
    for (u64 i = 0; i < map->capacity; ++i) {
      NamePair *pair = map->map + i;
      if (pair->name.ptr == NULL) {
        continue;
      }

      NamePair *dest = name_map_find(buffer, g.new_capacity, pair->name);
      *dest          = *pair;
    }

    free(map->map);
  }

  map->capacity = g.new_capacity;
  map->map      = buffer;
}

static void name_map_insert(NameMap *restrict map, StringView name, u64 tag) {
  if (name_map_full(map)) {
    name_map_grow(map);
  }

  NamePair *pair = name_map_find(map->map, map->capacity, name);
  map->size += 1;
  pair->name = name;
  pair->tag  = tag;
}

ControlFlowGraph control_flow_graph_create() {
  ControlFlowGraph cfg;
  cfg.graph = graph_create();
  cfg.names = name_map_create();
  return cfg;
}
void control_flow_graph_destroy(ControlFlowGraph *restrict cfg) {
  name_map_destroy(&cfg->names);
  graph_destroy(&cfg->graph);
}

void control_flow_graph_add_function(ControlFlowGraph *restrict cfg,
                                     StringView name) {
  u64 vertex = graph_add_vertex(&cfg->graph);
  name_map_insert(&cfg->names, name, vertex);
}

void control_flow_graph_add_call(ControlFlowGraph *restrict cfg,
                                 StringView source, StringView target) {
  NamePair *sf = name_map_find(cfg->names.map, cfg->names.capacity, source);
  if (sf == NULL) {
    PANIC("source function not in graph");
  }

  NamePair *tf = name_map_find(cfg->names.map, cfg->names.capacity, target);
  if (tf == NULL) {
    PANIC("target function not in graph");
  }

  graph_add_edge(&cfg->graph, sf->tag, tf->tag);
}

VertexList control_flow_graph_function_fanout(ControlFlowGraph *restrict cfg,
                                              StringView name) {
  NamePair *f = name_map_find(cfg->names.map, cfg->names.capacity, name);
  if (f == NULL) {
    PANIC("function not in graph");
  }

  return graph_vertex_fanout(&cfg->graph, f->tag);
}

VertexList control_flow_graph_function_fanin(ControlFlowGraph *restrict cfg,
                                             StringView name) {
  NamePair *f = name_map_find(cfg->names.map, cfg->names.capacity, name);
  if (f == NULL) {
    PANIC("function not in graph");
  }

  return graph_vertex_fanin(&cfg->graph, f->tag);
}