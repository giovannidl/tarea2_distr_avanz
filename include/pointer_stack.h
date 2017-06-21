//
// Created by gio on 18-06-17.
//

#ifndef TAREA2_POINTER_STACK_H
#define TAREA2_POINTER_STACK_H

typedef struct node {
    int* data;
    struct node *lower;
    int size;
} PointerStackNode;

typedef struct stack {
    int size;
    PointerStackNode *top;
} PointerStack;

PointerStack* stack_init(PointerStack* stack);

void stack_push(PointerStack* stack, int** array_pointer, int* pointer_size);

void stack_pop(PointerStack* stack, int** array_pointer, int* pointer_size);

void stack_peek(PointerStack* stack, int** array_pointer, int* pointer_size);

void stack_clear(PointerStack* stack);

#endif //TAREA2_POINTER_STACK_H
