//
// Created by gio on 18-06-17.
//
#include <stdio.h>
#include <mpi.h>
#include "pointer_stack_test.h"

int main(int argc, char **argv) {
    printf("Starting test...\n");

    int num_procs, my_id, an_id;
    int root_process = 0;

    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if(my_id == root_process) {
        // Test stack creation.
        test_stack_init();
    }

    MPI_Finalize();
    return 0;
}
