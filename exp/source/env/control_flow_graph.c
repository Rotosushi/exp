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
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/hash.h"
#include "utility/panic.h"

#define NAME_MAP_MAX_LOAD 0.75

static CFGNameMap name_map_create() {
  CFGNameMap map;
  map.size     = 0;
  map.capacity = 0;
  map.map      = NULL;
  return map;
}

static void name_map_destroy(CFGNameMap *restrict map) {
  map->size     = 0;
  map->capacity = 0;
  free(map->map);
  map->map = NULL;
}

static CFGPair *name_map_find(CFGPair *map, u64 capacity, StringView name) {
  u64 index = hash_cstring(name.ptr, name.length) % capacity;
  while (1) {
    CFGPair *pair = map + index;
    if ((pair->name.ptr == NULL) || (string_view_equality(name, pair->name))) {
      return pair;
    }
    index = (index + 1) % capacity;
  }
}

static CFGPair *name_map_find_name(CFGNameMap *restrict map, StringView name) {
  return name_map_find(map->map, map->capacity, name);
}

static bool name_map_full(CFGNameMap *restrict map) {
  u64 load_limit = (u64)floor((double)map->capacity * NAME_MAP_MAX_LOAD);
  return (map->size + 1) >= load_limit;
}

static void name_map_grow(CFGNameMap *restrict map) {
  Growth g        = array_growth(map->capacity, sizeof(CFGPair));
  CFGPair *buffer = callocate(g.new_capacity, sizeof(CFGPair));

  if (map->map != NULL) {
    for (u64 i = 0; i < map->capacity; ++i) {
      CFGPair *pair = map->map + i;
      if (pair->name.ptr == NULL) {
        continue;
      }

      CFGPair *dest = name_map_find(buffer, g.new_capacity, pair->name);
      *dest         = *pair;
    }

    free(map->map);
  }

  map->capacity = g.new_capacity;
  map->map      = buffer;
}

static void name_map_insert(CFGNameMap *restrict map, StringView name,
                            u64 vertex) {
  if (name_map_full(map)) {
    name_map_grow(map);
  }

  CFGPair *pair = name_map_find(map->map, map->capacity, name);
  map->size += 1;
  pair->name   = name;
  pair->vertex = vertex;
}

static CFGVertexMap vertex_map_create() {
  CFGVertexMap map;
  map.size     = 0;
  map.capacity = 0;
  map.map      = NULL;
  return map;
}

static void vertex_map_destroy(CFGVertexMap *restrict map) {
  map->capacity = 0;
  map->size     = 0;
  free(map->map);
  map->map = NULL;
}

static CFGPair *vertex_map_find(CFGPair *restrict buffer, u64 capacity,
                                u64 vertex) {
  u64 index = vertex % capacity;
  while (1) {
    CFGPair *pair = buffer + index;
    if ((pair->name.ptr == NULL) || (pair->vertex == vertex)) {
      return pair;
    }
    index = (index + 1) % capacity;
  }
}

static CFGPair *vertex_map_find_vertex(CFGVertexMap *restrict map, u64 vertex) {
  return vertex_map_find(map->map, map->capacity, vertex);
}

static bool vertex_map_full(CFGVertexMap *restrict map) {
  u64 load_limit = (u64)floor((double)map->capacity * NAME_MAP_MAX_LOAD);
  return (map->size + 1) >= load_limit;
}

static void vertex_map_grow(CFGVertexMap *restrict map) {
  Growth g        = array_growth(map->capacity, sizeof(CFGPair));
  CFGPair *buffer = callocate(g.new_capacity, sizeof(CFGPair));

  if (map->map != NULL) {
    for (u64 i = 0; i < map->capacity; ++i) {
      CFGPair *pair = map->map + i;
      if (pair->name.ptr == NULL) {
        continue;
      }

      CFGPair *dest = vertex_map_find(buffer, g.new_capacity, pair->vertex);
      *dest         = *pair;
    }

    free(map->map);
  }

  map->capacity = g.new_capacity;
  map->map      = buffer;
}

static void vertex_map_insert(CFGVertexMap *restrict map, u64 vertex,
                              StringView name) {
  if (vertex_map_full(map)) {
    vertex_map_grow(map);
  }

  CFGPair *pair = vertex_map_find(map->map, map->capacity, vertex);
  map->size += 1;
  pair->name   = name;
  pair->vertex = vertex;
}

static CFGBimap bimap_create() {
  CFGBimap map;
  map.names    = name_map_create();
  map.vertices = vertex_map_create();
  return map;
}

static void bimap_destroy(CFGBimap *restrict bimap) {
  name_map_destroy(&bimap->names);
  vertex_map_destroy(&bimap->vertices);
}

static void bimap_insert(CFGBimap *restrict bimap, StringView name,
                         u64 vertex) {
  name_map_insert(&bimap->names, name, vertex);
  vertex_map_insert(&bimap->vertices, vertex, name);
}

static CFGPair *bimap_find_name(CFGBimap *restrict bimap, StringView name) {
  return name_map_find_name(&bimap->names, name);
}

static CFGPair *bimap_find_vertex(CFGBimap *restrict bimap, u64 vertex) {
  return vertex_map_find_vertex(&bimap->vertices, vertex);
}

ControlFlowGraph control_flow_graph_create() {
  ControlFlowGraph cfg;
  cfg.graph = graph_create();
  cfg.bimap = bimap_create();
  return cfg;
}
void control_flow_graph_destroy(ControlFlowGraph *restrict cfg) {
  bimap_destroy(&cfg->bimap);
  graph_destroy(&cfg->graph);
}

void control_flow_graph_add_function(ControlFlowGraph *restrict cfg,
                                     StringView name) {
  u64 vertex = graph_add_vertex(&cfg->graph);
  bimap_insert(&cfg->bimap, name, vertex);
}

void control_flow_graph_add_call(ControlFlowGraph *restrict cfg,
                                 StringView source, StringView target) {
  CFGPair *sf = bimap_find_name(&cfg->bimap, source);
  if (sf == NULL) {
    PANIC("source function not in graph");
  }

  CFGPair *tf = bimap_find_name(&cfg->bimap, target);
  if (tf == NULL) {
    PANIC("target function not in graph");
  }

  graph_add_edge(&cfg->graph, sf->vertex, tf->vertex);
}

static NameList name_list_create() {
  NameList names;
  names.capacity = 0;
  names.size     = 0;
  names.list     = NULL;
  return names;
}

void name_list_destroy(NameList *restrict names) {
  names->capacity = 0;
  names->size     = 0;
  free(names->list);
  names->list = NULL;
}

static bool name_list_full(NameList *restrict names) {
  return (names->size + 1) >= names->capacity;
}

static void name_list_grow(NameList *restrict names) {
  Growth g         = array_growth(names->capacity, sizeof(StringView));
  StringView *list = reallocate(names->list, g.alloc_size);
  names->capacity  = g.new_capacity;
  names->list      = list;
}

static void name_list_insert(NameList *restrict names, StringView name) {
  if (name_list_full(names)) {
    name_list_grow(names);
  }

  names->list[names->size] = name;
  names->size += 1;
}

static NameList name_list_from_vertex_list(CFGBimap *restrict bimap,
                                           VertexList *restrict vertices) {
  NameList names = name_list_create();
  for (u64 i = 0; i < vertices->count; ++i) {
    u64 vertex = vertices->list[i];

    CFGPair *pair = bimap_find_vertex(bimap, vertex);
    if (pair == NULL) {
      PANIC("vertex not in map");
    }

    name_list_insert(&names, pair->name);
  }
  return names;
}

NameList control_flow_graph_function_fanout(ControlFlowGraph *restrict cfg,
                                            StringView name) {
  CFGPair *f = bimap_find_name(&cfg->bimap, name);
  if (f == NULL) {
    PANIC("function not in graph");
  }

  VertexList vertices = graph_vertex_fanout(&cfg->graph, f->vertex);
  NameList names      = name_list_from_vertex_list(&cfg->bimap, &vertices);
  vertex_list_destroy(&vertices);
  return names;
}

NameList control_flow_graph_function_fanin(ControlFlowGraph *restrict cfg,
                                           StringView name) {
  CFGPair *f = bimap_find_name(&cfg->bimap, name);
  if (f == NULL) {
    PANIC("function not in graph");
  }

  VertexList vertices = graph_vertex_fanin(&cfg->graph, f->vertex);
  NameList names      = name_list_from_vertex_list(&cfg->bimap, &vertices);
  vertex_list_destroy(&vertices);
  return names;
}