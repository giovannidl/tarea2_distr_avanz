//
// Created by gio on 18-06-17.
//
#include <stdio.h>
#include <mpi.h>
#include <memory.h>
#include "pointer_stack_test.h"

int checkArg(char** argv, int argc, char* value) {
    if(argc < 2)
        return 0;

    int i;
    for(i = 1; i < argc; i++)
        if(!strcmp(argv[i], value))
            return 1;

    return 0;
}

int main(int argc, char **argv) {
    printf("Starting test...\n");

    int num_procs, my_id, an_id;
    int root_process = 0;

    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if(my_id == root_process) {

        if(argc < 2 || checkArg(argv, argc, "stack") == 1) {
            // Stack Test
            printf("########################################\n");
            printf("############ Starting STACK ############\n");
            printf("########################################\n");
            int successful_tests = 1;
            test_stack_init(&successful_tests);
            test_push_pointer(&successful_tests);
            test_peek_stack(&successful_tests);
            test_push_two_pointers(&successful_tests);
            test_push_and_pop(&successful_tests);
            test_push_and_pop_twice(&successful_tests);
            test_alter_push_and_pop(&successful_tests);
            test_clear_stack(&successful_tests);

            if(successful_tests == 1)
                printf("All stack tests end successfully.\n");
            else
                printf("There are wrong tests.\n");
            printf("########################################\n");
            printf("############# Ending STACK #############\n");
            printf("########################################\n");
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}
