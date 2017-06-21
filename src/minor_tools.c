//
// Created by gio on 21-06-17.
//
#include <stdlib.h>
#include "../include/minor_tools.h"

int* get_missing_nodes(int* open_nodes, int open_nodes_size, int graph_size) {
    int* missing_nodes = (int*) malloc(sizeof(int) * (graph_size - open_nodes_size));
    int node_number, j, is_open, missing_count;
    missing_count = 0;
    for(node_number = 0; node_number < graph_size; node_number++) {
        is_open = 0;
        for(j = 0; j < open_nodes_size; j++) {
            if(open_nodes[j] == node_number)
                is_open = 1;
        }
        if(is_open == 0) {
            missing_nodes[missing_count] = node_number;
            missing_count++;
        }
    }
    return missing_nodes;
}