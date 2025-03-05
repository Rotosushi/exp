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

#include "adt/graph.h"

static bool list_contains(VertexList *restrict vl, u64 vertex) {
    for (u64 i = 0; i < vl->count; ++i) {
        if (vl->list[i] == vertex) { return 1; }
    }
    return 0;
}

int graph_tests([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    bool failure    = 0;
    SparseDigraph g = graph_create();

    u64 v0 = graph_add_vertex(&g);
    u64 v1 = graph_add_vertex(&g);
    u64 v2 = graph_add_vertex(&g);
    u64 v3 = graph_add_vertex(&g);

    /*
      v0 -> v1
      v0 -> v2
      v1 -> v3
      v2 -> v3
      v3 -> NULL
    */
    graph_add_edge(&g, v0, v1);
    graph_add_edge(&g, v0, v2);
    graph_add_edge(&g, v1, v3);
    graph_add_edge(&g, v2, v3);

    VertexList v0_fanout = graph_vertex_fanout(&g, v0);
    failure |= (v0_fanout.count != 2);
    failure |= (!list_contains(&v0_fanout, v1));
    failure |= (!list_contains(&v0_fanout, v2));
    vertex_list_destroy(&v0_fanout);

    VertexList v0_fanin = graph_vertex_fanin(&g, v0);
    failure |= (v0_fanin.count != 0);
    vertex_list_destroy(&v0_fanin);

    VertexList v1_fanout = graph_vertex_fanout(&g, v1);
    failure |= (v1_fanout.count != 1);
    failure |= (!list_contains(&v1_fanout, v3));
    vertex_list_destroy(&v1_fanout);

    VertexList v1_fanin = graph_vertex_fanin(&g, v1);
    failure |= (v1_fanin.count != 1);
    failure |= (!list_contains(&v1_fanin, v0));
    vertex_list_destroy(&v1_fanin);

    VertexList v2_fanout = graph_vertex_fanout(&g, v2);
    failure |= (v2_fanout.count != 1);
    failure |= (!list_contains(&v2_fanout, v3));
    vertex_list_destroy(&v2_fanout);

    VertexList v2_fanin = graph_vertex_fanin(&g, v2);
    failure |= (v2_fanin.count != 1);
    failure |= (!list_contains(&v2_fanin, v0));
    vertex_list_destroy(&v2_fanin);

    VertexList v3_fanout = graph_vertex_fanout(&g, v3);
    failure |= (v3_fanout.count != 0);
    vertex_list_destroy(&v3_fanout);

    VertexList v3_fanin = graph_vertex_fanin(&g, v3);
    failure |= (v3_fanin.count != 2);
    failure |= (!list_contains(&v3_fanin, v1));
    failure |= (!list_contains(&v3_fanin, v2));
    vertex_list_destroy(&v3_fanin);

    graph_destroy(&g);

    if (failure) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
