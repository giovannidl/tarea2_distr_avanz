//
// Created by gio on 21-06-17.
//

#ifndef TAREA2_MINOR_TOOLS_H
#define TAREA2_MINOR_TOOLS_H

/**
 * Get the missing nodes that have not been opened. Don't return the size of missing
 * nodes because is always (graph_size - open_nodes_size).
 * @param open_nodes
 * @param open_nodes_size
 * @param graph_size
 * @return
 */
int* get_missing_nodes(int* open_nodes, int open_nodes_size, int graph_size);

#endif //TAREA2_MINOR_TOOLS_H
