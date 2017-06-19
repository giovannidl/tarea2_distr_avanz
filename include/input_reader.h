//
// Created by gio on 14-06-17.
//

#ifndef TAREA2_INPUT_READER_H
#define TAREA2_INPUT_READER_H

/**
 * Read the data of the graph storage in the filename.
 * @param filename The file to read.
 * @param size Return the size of the graph.
 * @param costs Return the costs params of the graph.
 */
void reader_open(char* filename, int* size, int** costs);

void order_matrix(int** input, int* size, int (*matrix)[*size][*size]);

#endif //TAREA2_INPUT_READER_H
