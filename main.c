#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "include/input_reader.h"
#include "include/pointer_stack.h"
#include "include/minor_tools.h"



int main(int argc, char **argv) {
    int num_procs, my_id, i;
    int graph_size, graph_costs_length, best_cost;
    int *serialized_graph_costs, *best_road;
    int root_process = 0;

    PointerStack pending_stack;

    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc < 2) {
        if (my_id == root_process) {
            printf("Invalid number of argument. Expecting filename "
                           "with the graph parameters.\n");
        }
        MPI_Finalize();
        return 0;
    }

    // First we read the input file with the main process
    if (my_id == root_process) {
        reader_open(argv[1], &graph_size, &serialized_graph_costs);
    }

    // Share serialized_graph_costs with all process.

    MPI_Bcast(&graph_size, 1, MPI_INT, root_process, MPI_COMM_WORLD);

    graph_costs_length = (graph_size * (graph_size - 1)) / 2;

    // If we have more than one process, we transfer the serialized_graph_costs to the rest.
    if(num_procs > 1) {
        if(my_id != root_process)
            serialized_graph_costs = (int*) malloc(sizeof(int) * graph_costs_length);
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Bcast(serialized_graph_costs, graph_costs_length, MPI_INT, root_process,
                  MPI_COMM_WORLD);
    }

    // Create the matrix costs to get simples query
    int costs_matrix[graph_size][graph_size];
    order_matrix(&serialized_graph_costs, &graph_size, &costs_matrix);

    // Free the serialized_graph_costs because we won't use it anymore
    free(serialized_graph_costs);

    // Assign a high value to best_cost
    best_cost = 5000000;
    best_road = NULL;

    if(my_id == root_process) {
        // Initialize the stack with the started node zero
        int* start_node = (int*) malloc(sizeof(int));
        start_node[0] = 0;
        stack_init(&pending_stack);
        stack_push(&pending_stack, &start_node, 1);
    }

    if(num_procs == 1) {
        int *current_road, *missing_nodes, *new_road;
        int current_road_size, missing_nodes_size, partial_cost, current_cost;
        while(pending_stack.size > 0) {
            stack_pop(&pending_stack, &current_road, &current_road_size);
            // Get the current cost for the current opened nodes.
            current_cost = 0;
            for(i = 0; i < current_road_size - 1; i++) {
                current_cost += costs_matrix[current_road[i]][current_road[i + 1]];
            }
            // Get the missing nodes
            missing_nodes_size = graph_size - current_road_size;
            missing_nodes = get_missing_nodes(current_road, current_road_size, graph_size);
            for(i = 0; i < missing_nodes_size; i++) {
                partial_cost = current_cost;
                partial_cost +=
                        costs_matrix[current_road[current_road_size - 1]][missing_nodes[i]];
                // Only keep working with the road if the partial_cost is lower than the
                // current best cost.
                if(partial_cost < best_cost) {
                    // First create the new road.
                    new_road = (int*) malloc(sizeof(int) * (current_road_size + 1));
                    int j;
                    for(j = 0; j < current_road_size; j++)
                        new_road[j] = current_road[j];
                    new_road[current_road_size] = missing_nodes[i];

                    // Check if the node is a leaf. If it is, update the best cost and
                    // best road. If not, add the new road to pending_stack.
                    if(current_road_size == (graph_size - 1)) {
                        best_cost = partial_cost;
                        free(best_road);
                        best_road = new_road;
                    }
                    else {
                        stack_push(&pending_stack, &new_road, current_road_size + 1);
                    }
                }
            }

            // Release the memory
            free(missing_nodes);
            free(current_road);
        }
    }

    if(best_road != NULL) {
        printf("The best road has been found.\n");
        printf("Best cost: %i.\n", best_cost);
        printf("Best road:");
        for(i = 0; i < graph_size; i++){
            printf(" %i", best_road[i]);
        }
        printf(".\n");
    }
    else {
        printf("The best road has not been found.\n");
    }

    if(my_id == root_process) {
        free(best_road);
    }

    MPI_Finalize();
    return 0;
}