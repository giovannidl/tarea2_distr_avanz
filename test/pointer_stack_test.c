//
// Created by gio on 18-06-17.
//
#include <stdio.h>
#include "../include/pointer_stack.h"

void test_stack_init(void) {
    // Initialize an empty stack.
    printf("###### Test stack creation. #####\n");
    PointerStack stack;
    stack_init(&stack);
    printf("Tamaño inicial del stack: %i\n", stack.size);
    if(stack.top == NULL)
        printf("Top del stack es nulo\n");
    else
        printf("Top del stack no es nulo\n");
    printf("\n");
}