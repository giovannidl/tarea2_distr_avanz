#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "include/input_reader.h"
#include "include/pointer_stack.h"
#include "include/minor_tools.h"



int main(int argc, char **argv) {
    int num_procs, my_id, i;
    int graph_size, graph_costs_length, best_cost;
    int* serialized_graph_costs;
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

        MPI_Bcast(serialized_graph_costs, graph_costs_length, MPI_INT, root_process, MPI_COMM_WORLD);
    }

    // Create the matrix costs to get simples query
    int costs_matrix[graph_size][graph_size];
    order_matrix(&serialized_graph_costs, &graph_size, &costs_matrix);

    // Free the serialized_graph_costs because we won't use it anymore
    free(serialized_graph_costs);

    // Assign a high value to best_cost
    best_cost = 5000000;

    if(my_id == root_process) {
        // Initialize the stack with the started node zero
        int* start_node = (int*) malloc(sizeof(int));
        stack_init(&pending_stack);
        stack_push(&pending_stack, &start_node, 1);
    }

    if(num_procs == 1) {
        int* current_road;
        int current_road_size;
        while(pending_stack.size > 0) {
            stack_pop(&pending_stack, &current_road, &current_road_size);

        }
    }

    MPI_Finalize();
    return 0;
}