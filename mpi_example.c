#include <stdio.h>
#include <mpi.h>
#include <sys/time.h>

#define max_rows 10000000
#define send_data_tag 2001
#define return_data_tag 2002

int array[max_rows];
int array2[max_rows];

int main(int argc, char **argv) {
    long int sum, partial_sum;
    MPI_Status status;
    int my_id, root_process, i, num_rows, num_procs, an_id, num_rows_to_receive, avg_rows_per_process,
            sender, num_rows_received, start_row, end_row, num_rows_to_send;

    struct timeval start, end;

    gettimeofday(&start, NULL);

    root_process = 0;

    // Now replicate this process to create parallel processes.
    MPI_Init(&argc, &argv);

    // Find out my process ID, and how many processes where started
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (my_id == root_process) {
        // Root process

        num_rows = 1000000;
        avg_rows_per_process = num_rows / num_procs;
        if (avg_rows_per_process * num_procs < num_rows)
            avg_rows_per_process++;

        // Initialize an array
        for (i = 0; i < num_rows; i++) {
            array[i] = i;
        }

        // Distribute a portion of the vector to each child process
        for (an_id = 1; an_id < num_procs; an_id++) {
            start_row = an_id * avg_rows_per_process;
            end_row = (an_id + 1) * avg_rows_per_process - 1;
            if (num_rows <= end_row)
                end_row = num_rows - 1;

            num_rows_to_send = end_row - start_row + 1;

            MPI_Send(&num_rows_to_send, 1, MPI_INT, an_id, send_data_tag, MPI_COMM_WORLD);
            MPI_Send(&array[start_row], num_rows_to_send, MPI_INT, an_id, send_data_tag, MPI_COMM_WORLD);
        }

        // Calculate the sum of the values in the segment assigned to the root process

        sum = 0;
        for (i = 0; i < avg_rows_per_process; i++) {
            sum += array[i];
        }

        printf("sum %ld calculated by root process\n", sum);

        // Finally collect the partial sums from the slave processes, print them and add them to the grand sum.
        for (an_id = 1; an_id < num_procs; an_id++) {
            MPI_Recv(&partial_sum, 1, MPI_LONG, MPI_ANY_SOURCE, return_data_tag, MPI_COMM_WORLD, &status);

            sender = status.MPI_SOURCE;

            printf("Partial sum %ld returned from process %i\n", partial_sum, sender);

            sum += partial_sum;
        }

        printf("The grand total is: %ld\n", sum);

        gettimeofday(&end, NULL);
        long int elapsed_time = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
        printf("Tiempo es: %ld\n", elapsed_time);
    } else {
        // Children processes
        MPI_Recv(&num_rows_to_receive, 1, MPI_INT, root_process, send_data_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&array2, num_rows_to_receive, MPI_INT, root_process, send_data_tag, MPI_COMM_WORLD, &status);

        num_rows_received = num_rows_to_receive;

        // Calculate the sum of my portion of the array
        partial_sum = 0;
        for (i = 0; i < num_rows_received; i++) {
            partial_sum += array2[i];
        }

        // Finally send my partial sum to the root process
        MPI_Send(&partial_sum, 1, MPI_LONG, root_process, return_data_tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}