//
// Created by gio on 14-06-17.
//

#include "../include/input_reader.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Read the data of the graph storage in the filename.
 * @param filename The file to read.
 * @param size Return the size of the graph.
 * @param costs Return the costs params of the graph.
 */
void reader_open(char* filename, int* size, int** costs) {
    FILE *fp;
    fp = fopen(filename, "r");

    // Obtain the size of the graph.
    fscanf(fp, "%i", size);

    // Allocate the memory for the costs params.
    int costs_length = (*size * (*size - 1)) / 2;
//    int* input_costs;
    (*costs) = (int*) malloc(sizeof(int) * costs_length);

    // Iterate for all the costs and assign to costs array.
    int costs_aux;
    int cost_pos = 0;
    while(fscanf(fp, "%i", &costs_aux) != EOF) {
        (*costs)[cost_pos] = costs_aux;
        cost_pos++;
    }

    // Closing the file
    fclose(fp);
}

void order_matrix(int** input, int* size, int (*matrix)[*size][*size]) {
    /* Iterate input_costs like it was a matrix and assign the value in the
     * matrix.
    */
    int i, j, output_pos, input_diff, input_pos;
    for(i = 0; i < *size; i++) {
        input_diff = (i + 1) * (i + 2) / 2;
        for(j = 0; j < *size; j++) {
            if(i < j) {
                output_pos = i * (*size) + j;
                input_pos = output_pos - input_diff;

                (*matrix)[i][j] = (*input)[input_pos];
            }
            else if(i > j) {
                (*matrix)[i][j] = (*matrix)[j][i];
            }
            else {
                (*matrix)[i][j] = 0;
            }
        }
    }
}