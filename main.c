#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/time.h>
#include "include/input_reader.h"
#include "include/pointer_stack.h"
#include "include/minor_tools.h"

#define send_opened_tag 2001
#define send_new_tag 2002
#define send_finish_tag 2003

int main(int argc, char **argv) {
    int num_procs, my_id, i;
    int graph_size, graph_costs_length, best_cost;
    int *serialized_graph_costs, *best_road;
    int root_process = 0;

    struct timeval startTime, endTime;

    gettimeofday(&startTime, NULL);

    PointerStack pending_stack;

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

    // If we have more than one process, we transfer the serialized_graph_costs
    // to the rest.
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
        // #####################################################
        // ################## SEQUENTIAL FORM ##################
        // #####################################################
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
                partial_cost += costs_matrix[current_road[current_road_size - 1]]
                        [missing_nodes[i]];
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
    else {
        // #####################################################
        // ################# DISTRIBUTED FORM ##################
        // #####################################################

        // Variable to set a process have finished his job.
        // Root process only use for auxiliar purpose.
        int jobFinished = 1;
        // Array with 1 for the process that are waiting orders and 0 with the process
        // that are working.
        int wait_for_orders[num_procs];
        // Asign waiting orders to all other process.
        // Other process are in charge of changing their own wait orders when they finish
        // their job.
        wait_for_orders[root_process] = 0;
        for(i = 1; i < num_procs; i ++)
            wait_for_orders[i] = 1;
        MPI_Status status;
        MPI_Status receiveStatus;
        MPI_Status finishedStatus;

        if(my_id == root_process) {
            // ################# ROOT PROCESS ###################
            int* send_road, *receive_road;
            int send_road_size, receive_road_size, source_process;

            MPI_Request requestNewNode;
            MPI_Request requestFinishedProcs;
            // Flag if receive_road_size have been received
            int sizeFlag = 0;
            int finishFlag = 0;

            // Asynchronous receive to check if some process have sent data to root process
            MPI_Irecv(&receive_road_size, 1, MPI_INT, MPI_ANY_SOURCE, send_new_tag,
                      MPI_COMM_WORLD, &requestNewNode);

            // Asynchronous receive to check if some process have finished their job.
            MPI_Irecv(&jobFinished, 1, MPI_INT, MPI_ANY_SOURCE, send_finish_tag,
                      MPI_COMM_WORLD, &requestFinishedProcs);

            while(pending_stack.size > 0 || !wait_for_orders[1] || sizeFlag) {
                // Check if there are process waiting for opened nodes.
                if(pending_stack.size > 0 && wait_for_orders[1]) {
                    printf("StackPop.\n");
                    stack_pop(&pending_stack, &send_road, &send_road_size);
                    MPI_Send(&send_road_size, 1, MPI_INT, 1, send_opened_tag,
                             MPI_COMM_WORLD);
                    MPI_Send(send_road, send_road_size, MPI_INT, 1, send_opened_tag,
                             MPI_COMM_WORLD);

                    free(send_road);
                    wait_for_orders[1] = 0;
                }

                // Check if there are new nodes waiting.
                if(sizeFlag) {
                    printf("SizeFlag is up.\n");
                    if(receiveStatus.MPI_SOURCE != -1) {
                        printf("receiveStatus.MPI_SOURCE: %i.\n",
                               receiveStatus.MPI_SOURCE);
                        source_process = receiveStatus.MPI_SOURCE;
                        printf("Received receive_road_size: %i; Origin process: %i; "
                                       "process: %i\n",
                               receive_road_size, source_process, my_id);

                        receive_road = (int*) malloc(sizeof(int) * receive_road_size);
                        MPI_Recv(receive_road, receive_road_size, MPI_INT, source_process,
                                 send_new_tag, MPI_COMM_WORLD, &status);
                        printf("Received receive_road[0]: %i; Or. procs: %i; "
                                       "process: %i\n",
                               receive_road[0], source_process, my_id);
                        if(receive_road_size > 1)
                            printf("Received receive_road[1]: %i; Or. procs: %i; "
                                           "process: %i\n",
                                   receive_road[1], source_process, my_id);

                        // Call the asynchronous receive to check again new nodes.
                        MPI_Irecv(&receive_road_size, 1, MPI_INT, MPI_ANY_SOURCE,
                                  send_new_tag, MPI_COMM_WORLD, &requestNewNode);

                        // TODO replace this with stack_push
                        free(receive_road);
                    }
                    else {
                        printf("receiveStatus.MPI_SOURCE: -1 with sizeFlag: 1.\n");
                    }
                    sizeFlag = 0;
                }

                // Check if there are process who are finished their job
                if(finishFlag) {
                    printf("FinishFlag is up.\n");
                    if(finishedStatus.MPI_SOURCE != -1) {
                        printf("finishedStatus.MPI_SOURCE: %i.\n",
                               finishedStatus.MPI_SOURCE);
                        // Set the wait_for_orders flag for the source process.
                        wait_for_orders[finishedStatus.MPI_SOURCE] = 1;
                        // Asynchronous receive to check if some process have finished
                        // their job.
                        MPI_Irecv(&jobFinished, 1, MPI_INT, MPI_ANY_SOURCE,
                                  send_finish_tag, MPI_COMM_WORLD, &requestFinishedProcs);
                    }
                    else {
                        printf("finishedStatus.MPI_SOURCE: -1 with finishFlag: 1.\n");
                    }
                    finishFlag = 0;
                }

                // Test the asynchronous receives
                MPI_Test(&requestNewNode, &sizeFlag, &receiveStatus);
                MPI_Test(&requestFinishedProcs, &finishFlag, &finishedStatus);
                printf("Final sizeFlag: %i finishFlag: %i.\n", sizeFlag, finishFlag);
            }

//            while(pending_stack.size > 0) {
//                // Give the node to some
//
//            }
        }
        else {
            // ################## WORK PROCESS ##################

            int *current_road, *missing_nodes, *new_road;
            int current_road_size, missing_nodes_size, partial_cost, current_cost;

            int new_road_size;
            if(my_id == 1) {
                MPI_Recv(&current_road_size, 1, MPI_INT, root_process, send_opened_tag,
                         MPI_COMM_WORLD, &status);
                printf("current_road_size: %i; process: %i.\n", current_road_size, my_id);
                current_road = (int*) malloc(sizeof(int) * current_road_size);
                MPI_Recv(current_road, current_road_size, MPI_INT, root_process,
                         send_opened_tag, MPI_COMM_WORLD, &status);

                printf("current_road[0]: %i; process: %i.\n", current_road[0], my_id);

                current_cost = 0;
                for(i = 0; i < current_road_size - 1; i++) {
                    current_cost += costs_matrix[current_road[i]][current_road[i + 1]];
                }
                // Get the missing nodes
                missing_nodes_size = graph_size - current_road_size;
                missing_nodes = get_missing_nodes(current_road, current_road_size,
                                                  graph_size);
                for(i = 0; i < missing_nodes_size; i++) {
                    partial_cost = current_cost;
                    partial_cost += costs_matrix[current_road[current_road_size - 1]]
                    [missing_nodes[i]];
                    // Only keep working with the road if the partial_cost is lower
                    // than the current best cost.
                    if(partial_cost < best_cost) {
                        // First create the new road.
                        new_road_size = current_road_size + 1;
                        new_road = (int*) malloc(sizeof(int) * new_road_size);
                        int j;
                        for(j = 0; j < current_road_size; j++)
                            new_road[j] = current_road[j];
                        new_road[current_road_size] = missing_nodes[i];

                        MPI_Send(&new_road_size, 1, MPI_INT, root_process, send_new_tag,
                                 MPI_COMM_WORLD);
                        MPI_Send(new_road, new_road_size, MPI_INT, root_process,
                                 send_new_tag, MPI_COMM_WORLD);

                        free(new_road);
//                        // Check if the node is a leaf. If it is, update the best cost
//                        // and best road. If not, add the new road to pending_stack.
//                        if(current_road_size == (graph_size - 1)) {
//                            best_cost = partial_cost;
//                            free(best_road);
//                            best_road = new_road;
//                        }
//                        else {
//                            stack_push(&pending_stack, &new_road, current_road_size + 1);
//                        }
                    }
                }

                // Release the memory
                free(missing_nodes);
                free(current_road);

                // When the worker process finish his job send final message
                MPI_Send(&jobFinished, 1, MPI_INT, root_process, send_finish_tag,
                         MPI_COMM_WORLD);
            }
        }
    }

    if(my_id == root_process) {
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
        free(best_road);

        gettimeofday(&endTime, NULL);
        long int time_seconds = (endTime.tv_sec - startTime.tv_sec);
        long int time_milliseconds = (endTime.tv_usec - startTime.tv_usec) / 1000;
        if(time_milliseconds < 0) {
            time_seconds--;
            time_milliseconds += 1000;
        }
        printf("Elapsed time: %ld seconds and %ld milliseconds.\n", time_seconds,
               time_milliseconds);
    }
    printf("Process: %i Finalize.\n", my_id);

    MPI_Finalize();
    return 0;
}