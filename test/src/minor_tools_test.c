//
// Created by gio on 21-06-17.
//
#include <stdio.h>
#include <stdlib.h>
#include "../include/minor_tools_test.h"
#include "../include/test_tools.h"
#include "../../include/minor_tools.h"

void test_get_missing_nodes(int* successful_tests) {
    printf("###### Test get missing nodes method ######\n");
    int graph_size = 5;
    int open_nodes_size = 2;
    int* open_nodes = (int*) malloc(sizeof(int) * open_nodes_size);
    open_nodes[0] = 0;
    open_nodes[1] = 3;

    int* missing_nodes = get_missing_nodes(open_nodes, open_nodes_size, graph_size);

    printIntValidate("MissingNodes[0]: %i.", missing_nodes[0], 1, successful_tests);
    printIntValidate("MissingNodes[0]: %i.", missing_nodes[1], 2, successful_tests);
    printIntValidate("MissingNodes[0]: %i.", missing_nodes[2], 4, successful_tests);

    printf("\n");
    free(missing_nodes);
    free(open_nodes);
}