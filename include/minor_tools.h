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

/**
 * Return 1 if all int in the array are 1, 0 otherwise.
 * @param num_procs
 * @param wait_for_orders
 * @return
 */
int all_true(int *num_procs, int (*wait_for_orders)[*num_procs]);

/**
 * Return all positions that have value 1.
 * @param num_procs
 * @param wait_for_orders
 * @return
 */
void get_true_pos(int *num_procs, int (*wait_for_orders)[*num_procs], int **selected_procs,
                 int *selected_procs_count);

#endif //TAREA2_MINOR_TOOLS_H
