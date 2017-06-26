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
#define send_program_tag 2004
#define send_best_cost_tag 2005

int main(int argc, char **argv) {
    int num_procs, my_id, i;
    int graph_size, graph_costs_length, best_cost;
    int *serialized_graph_costs, *best_road;
    int root_process = 0;
    int total_nodes_opened = 0, nodes_opened_umbral = 10000;

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
        stack_push(&pending_stack, &start_node, 1, 0);
    }

    if(num_procs == 1) {
        // #####################################################
        // ################## SEQUENTIAL FORM ##################
        // #####################################################
        int *current_road, *missing_nodes, *new_road;
        int current_road_size, missing_nodes_size, partial_cost, current_cost;
        while(pending_stack.size > 0) {
            stack_pop(&pending_stack, &current_road, &current_road_size, &current_cost);

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
                        stack_push(&pending_stack, &new_road, current_road_size + 1,
                                   partial_cost);
                    }
                }
            }

            total_nodes_opened++;

            // Release the memory
            free(missing_nodes);
            free(current_road);
        }
    }
    else {
        // #####################################################
        // ################# DISTRIBUTED FORM ##################
        // #####################################################

        // Flag to terminate the program in all process.
        int program_flag = 0;
        // Variable to set a process have finished his job.
        // Root process only use for auxiliar purpose.
        int job_finished = 1;
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
        MPI_Status receive_status;
        MPI_Status finished_status;

        if(my_id == root_process) {
            // ################# ROOT PROCESS ###################
            int* send_road, *receive_road, *selected_procs;
            int send_road_size, receive_road_size, source_process, receive_road_cost,
                    selected_procs_count, send_road_cost;

            MPI_Request request_new_node;
            MPI_Request request_finished_procs;
            // Flag if receive_road_size have been received
            int size_flag = 0;
            int finish_flag = 0;

            // Asynchronous receive to check if some process have sent data to root process
            MPI_Irecv(&receive_road_size, 1, MPI_INT, MPI_ANY_SOURCE, send_new_tag,
                      MPI_COMM_WORLD, &request_new_node);

            // Asynchronous receive to check if some process have finished their job.
            MPI_Irecv(&job_finished, 1, MPI_INT, MPI_ANY_SOURCE, send_finish_tag,
                      MPI_COMM_WORLD, &request_finished_procs);

            while(pending_stack.size > 0 || !all_true(&num_procs, &wait_for_orders)
                  || size_flag) {
                // Check if there are process waiting for opened nodes.
                if(pending_stack.size > 0 && all_true(&num_procs, &wait_for_orders)) {
                    get_true_pos(&num_procs, &wait_for_orders, &selected_procs,
                                 &selected_procs_count);
                    // Iterate for all the waiting process and give them opened nodes
                    for(i = 0; i < selected_procs_count && pending_stack.size > 0; i++) {
//                        printf("StackPop. Selected procs: %i.\n", selected_procs[i]);
                        stack_pop(&pending_stack, &send_road, &send_road_size,
                                  &send_road_cost);
                        if(send_road_cost < best_cost) {
                            MPI_Send(&send_road_size, 1, MPI_INT, selected_procs[i],
                                     send_opened_tag, MPI_COMM_WORLD);
                            MPI_Send(send_road, send_road_size, MPI_INT, selected_procs[i],
                                     send_opened_tag, MPI_COMM_WORLD);
                            wait_for_orders[selected_procs[i]] = 0;
                        }

                        free(send_road);
                    }

                    free(selected_procs);
                }

                // Check if there are new nodes waiting.
                if(size_flag) {
//                    printf("SizeFlag is up.\n");
                    if(receive_status.MPI_SOURCE != -1) {
//                        printf("receive_status.MPI_SOURCE: %i.\n",
//                               receive_status.MPI_SOURCE);
                        source_process = receive_status.MPI_SOURCE;
//                        printf("Received receive_road_size: %i; Origin process: %i; "
//                                       "process: %i\n",
//                               receive_road_size, source_process, my_id);

                        receive_road = (int*) malloc(sizeof(int) * receive_road_size);
                        MPI_Recv(receive_road, receive_road_size, MPI_INT, source_process,
                                 send_new_tag, MPI_COMM_WORLD, &status);

//                        printf("Received receive_road: ");
//                        for(i = 0; i < receive_road_size; i++)
//                            printf("%i ", receive_road[i]);
//                        printf("; Or. procs: %i; process: %i\n", source_process, my_id);

                        receive_road_cost = 0;
                        for(i = 0; i < receive_road_size - 1; i++) {
                            receive_road_cost += costs_matrix[receive_road[i]]
                            [receive_road[i + 1]];
                        }

                        // Check if the node is a leaf. If it is, update the best cost
                        // and best road. If not, add the new road to pending_stack.
                        if(receive_road_size == graph_size &&
                                receive_road_cost < best_cost) {
                            if(receive_road_cost < best_cost) {
                                best_cost = receive_road_cost;
                                free(best_road);
                                best_road = receive_road;

                                // Send to all process the new best cost.
//                                printf("Update best cost %i to all process.\n",
//                                  best_cost);
                                for(i = 1; i < num_procs; i++)
                                    MPI_Send(&best_cost, 1, MPI_INT, i, send_best_cost_tag,
                                             MPI_COMM_WORLD);
                            }
                        }
                        else {
                            stack_push(&pending_stack, &receive_road, receive_road_size,
                                       receive_road_cost);
                        }

                        // Call the asynchronous receive to check again new nodes.
                        MPI_Irecv(&receive_road_size, 1, MPI_INT, MPI_ANY_SOURCE,
                                  send_new_tag, MPI_COMM_WORLD, &request_new_node);

                        total_nodes_opened++;
//                        if(!(total_nodes_opened % nodes_opened_umbral))
//                            printf("total_nodes_opened: %i.\n", total_nodes_opened);
                    }
                    else {
                        printf("receive_status.MPI_SOURCE: -1 with sizeFlag: 1.\n");
                    }
                    size_flag = 0;
                }

                // Check if there are process who have finished their job
                if(finish_flag) {
//                    printf("FinishFlag is up.\n");
                    if(finished_status.MPI_SOURCE != -1) {
//                        printf("finishedStatus.MPI_SOURCE: %i.\n",
//                               finished_status.MPI_SOURCE);
                        // Set the wait_for_orders flag for the source process.
                        wait_for_orders[finished_status.MPI_SOURCE] = 1;
                        // Asynchronous receive to check if some process have finished
                        // their job.
                        MPI_Irecv(&job_finished, 1, MPI_INT, MPI_ANY_SOURCE,
                                  send_finish_tag, MPI_COMM_WORLD,
                                  &request_finished_procs);
                    }
                    else {
                        printf("finishedStatus.MPI_SOURCE: -1 with finishFlag: 1.\n");
                    }
                    finish_flag = 0;
                }

                // Test the asynchronous receives
                MPI_Test(&request_new_node, &size_flag, &receive_status);
                MPI_Test(&request_finished_procs, &finish_flag, &finished_status);
//                printf("Final sizeFlag: %i finishFlag: %i.\n", size_flag, finish_flag);
            }

            // Send the message to all process to end.
//            printf("Calling all process to end.\n");
            for(i = 1; i < num_procs; i++)
                MPI_Send(&job_finished, 1, MPI_INT, i, send_program_tag, MPI_COMM_WORLD);
        }
        else {
            // ################## WORK PROCESS ##################

            int *current_road, *missing_nodes, *new_road;
            int current_road_size, missing_nodes_size, partial_cost, current_cost;

            int new_road_size;
            MPI_Request request_opened;
            MPI_Request request_program;
            MPI_Request request_best_cost;
            MPI_Status program_status;
            MPI_Status best_cost_status;
            int opened_flag = 0;
            int best_cost_flag = 0;

            // Asynchronous receive to know where are opened nodes pending.
            MPI_Irecv(&current_road_size, 1, MPI_INT, root_process, send_opened_tag,
                      MPI_COMM_WORLD, &request_opened);

            // Asynchronous receive to know when the program is finished.
            MPI_Irecv(&job_finished, 1, MPI_INT, root_process, send_program_tag,
                      MPI_COMM_WORLD, &request_program);

            // Asyncrhonous receive to know when the best cost have changed
            MPI_Irecv(&best_cost, 1, MPI_INT, root_process, send_best_cost_tag,
                      MPI_COMM_WORLD, &request_best_cost);

            while(!program_flag) {
                if(opened_flag) {
//                    printf("current_road_size: %i; process: %i.\n", current_road_size,
//                           my_id);
                    current_road = (int*) malloc(sizeof(int) * current_road_size);
                    MPI_Recv(current_road, current_road_size, MPI_INT, root_process,
                             send_opened_tag, MPI_COMM_WORLD, &receive_status);

//                    printf("Received current_road: ");
//                    for(i = 0; i < current_road_size; i++)
//                        printf("%i ", current_road[i]);
//                    printf("; process: %i\n", my_id);

                    current_cost = 0;
                    for(i = 0; i < current_road_size - 1; i++) {
                        current_cost += costs_matrix[current_road[i]][current_road[i + 1]];
                    }

                    if(current_cost < best_cost) {
                        // Get the missing nodes
                        missing_nodes_size = graph_size - current_road_size;
                        missing_nodes = get_missing_nodes(current_road, current_road_size,
                                                          graph_size);
                        for(i = 0; i < missing_nodes_size; i++) {
                            partial_cost = current_cost;
                            partial_cost += costs_matrix
                                [current_road[current_road_size - 1]][missing_nodes[i]];

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

                                MPI_Send(&new_road_size, 1, MPI_INT, root_process,
                                         send_new_tag, MPI_COMM_WORLD);
                                MPI_Send(new_road, new_road_size, MPI_INT, root_process,
                                         send_new_tag, MPI_COMM_WORLD);

                                free(new_road);
                            }
                        }

                        // Release the memory
                        free(missing_nodes);
                        free(current_road);

                        // When the worker process finish his job send final message
                        MPI_Send(&job_finished, 1, MPI_INT, root_process, send_finish_tag,
                                 MPI_COMM_WORLD);

                        // Call again the asynchronous receive for opened nodes.
                        MPI_Irecv(&current_road_size, 1, MPI_INT, root_process,
                                  send_opened_tag, MPI_COMM_WORLD, &request_opened);

                        total_nodes_opened++;
                    }
                }

                if(best_cost_flag) {
                    // If the best value has been updated, create the asynchronous receive
                    // again.
                    MPI_Irecv(&best_cost, 1, MPI_INT, root_process, send_best_cost_tag,
                              MPI_COMM_WORLD, &request_best_cost);
                }

                // Test the asynchronous receives
                MPI_Test(&request_opened, &opened_flag, &receive_status);
                MPI_Test(&request_program, &program_flag, &program_status);
                MPI_Test(&request_best_cost, &best_cost_flag, &best_cost_status);

//                printf("Final opened_flag: %i program_flag: %i. Process: %i.\n",
//                       opened_flag, program_flag, my_id);
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
            printf("Total nodes opened: %i.\n", total_nodes_opened);
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
    printf("Process: %i Finalize. Nodes opened: %i.\n", my_id, total_nodes_opened);

    MPI_Finalize();
    return 0;
}