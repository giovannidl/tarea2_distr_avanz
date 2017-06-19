#include <stdio.h>
#include <sys/time.h>

#define max_rows 10000000

int array[max_rows];

int main(int argc, char **argv) {
    int i, num_rows = 1000000;
    long int sum;

    struct timeval start, end;

    gettimeofday(&start, NULL);

    for (i = 0; i < num_rows; i++) {
        array[i] = i;
    }

    sum = 0;
    for (i = 0; i < num_rows; i++) {
        sum += array[i];
    }

    printf("The grand total is: %ld\n", sum);

    gettimeofday(&end, NULL);
    long int elapsed_time = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    printf("Tiempo es: %ld\n", elapsed_time);

    return 0;
}