//
// Created by gio on 18-06-17.
//
#include <stdio.h>
#include <stdlib.h>
#include "../include/pointer_stack.h"

int printIntValidate(char* message, int answer, int correct_answer, int* successful_tests) {
    printf(message, answer);
    if(answer == correct_answer)
        printf(" Correct\n");
    else {
        printf(" Wrong\n");
        *successful_tests = 0;
    }
}

void test_stack_init(int* successful_tests) {
    // Initialize an empty stack.
    printf("###### Test stack creation. ######\n");
    PointerStack stack;
    stack_init(&stack);

    printIntValidate("Initial size of stack: %i.", stack.size, 0, successful_tests);

    if(stack.top == NULL)
        printf("Top of stack is null. Correct\n");
    else {
        printf("Top of stack is not null. Wrong\n");
        *successful_tests = 0;
    }
    printf("\n");
}

void test_push_pointer(int* successful_tests) {
    printf("###### Test push one pointer ######\n");
    PointerStack stack;
    int example_size;
    int* example;

    stack_init(&stack);
    example_size = 3;
    example = (int*) malloc(sizeof(int) * example_size);
    example[0] = 4;
    example[1] = 5;
    example[2] = 6;

    stack_push(&stack, &example, &example_size);

    printIntValidate("Stack Size: %i.", stack.size, 1, successful_tests);
    if(stack.top == NULL) {
        printf("Stack top is null. Wrong\n");
        *successful_tests = 0;
    }
    else {
        printf("Stack top is not null. Correct\n");
        printIntValidate("Stack top size: %i.", stack.top->size, 3, successful_tests);
    }

    if(stack.top->data == example)
        printf("Top data is the same as the example. Correct\n");
    else {
        printf("Top data is not the same as the example. Wrong\n");
        *successful_tests = 0;
    }

    if(stack.top->lower == NULL)
        printf("Top lower node is null. Correct\n");
    else {
        printf("Top lower node is not null. Wrong\n");
        *successful_tests = 0;
    }

    printf("\n");
    free(example);
}

void test_peek_stack(int* successful_tests) {
    printf("###### Test peek a stack with one pointer ######\n");
    PointerStack stack;
    int example_size, result_size;
    int *example, *pop_result;

    stack_init(&stack);
    example_size = 3;
    example = (int*) malloc(sizeof(int) * example_size);
    example[0] = 4;
    example[1] = 5;
    example[2] = 6;

    stack_push(&stack, &example, &example_size);
    stack_peek(&stack, &pop_result, &result_size);

    if(pop_result == example)
        printf("PopResult is the same as the example. Correct\n");
    else {
        printf("PopResult is not the same as the example. Wrong\n");
        *successful_tests = 0;
    }

    printIntValidate("Size of pop_result: %i.", result_size, example_size, successful_tests);
    printIntValidate("Size of stack: %i.", stack.size, 1, successful_tests);

    if(stack.top == NULL) {
        printf("Top of stack is null. Wrong\n");
        *successful_tests = 0;
    }
    else {
        printf("Top of stack is not null. Correct\n");
        if(stack.top->data == example)
            printf("Top data is the same as the example. Correct\n");
        else {
            printf("Top data is not the same as the example. Wrong\n");
            *successful_tests = 0;
        }
    }
    printf("\n");

    if(example != pop_result)
        free(pop_result);
    free(example);
    free(stack.top);
}

void test_push_two_pointers(int* successful_tests) {
    printf("###### Test push two pointers ######\n");
    PointerStack stack;
    int example_size, second_example_size;
    int *example, *second_example;

    stack_init(&stack);
    example_size = 3;
    example = (int*) malloc(sizeof(int) * example_size);
    example[0] = 4;
    example[1] = 5;
    example[2] = 6;

    second_example_size = 4;
    second_example = (int*) malloc(sizeof(int) * second_example_size);
    second_example[0] = 11;
    second_example[1] = 12;
    second_example[2] = 13;
    second_example[3] = 14;

    stack_push(&stack, &example, &example_size);
    stack_push(&stack, &second_example, &second_example_size);

    printIntValidate("Stack Size: %i.", stack.size, 2, successful_tests);
    if(stack.top == NULL) {
        printf("Stack top is null. Wrong\n");
        *successful_tests = 0;
    }
    else {
        printf("Stack top is not null. Correct\n");
        printIntValidate("Stack top size: %i.", stack.top->size, 4, successful_tests);
    }

    if(stack.top->data == second_example)
        printf("Top data is the same as the second_example. Correct\n");
    else {
        printf("Top data is not the same as the second_example. Wrong\n");
        *successful_tests = 0;
    }

    if(stack.top->lower == NULL) {
        printf("Top lower node is null. Wrong\n");
        *successful_tests = 0;
    }
    else {
        printf("Top lower node is not null. Correct\n");
        if(stack.top->lower->data == example)
            printf("Lower node data is the same as the example. Correct\n");
        else {
            printf("Lower node data is not the same as the example. Wrong\n");
            *successful_tests = 0;
        }
    }

    printf("\n");

    free(example);
    free(second_example);
}

void test_push_and_pop(int* successful_tests) {
    printf("###### Test push and pop one pointer ######\n");
    PointerStack stack;
    int example_size, result_size;
    int *example, *pop_result;

    stack_init(&stack);
    example_size = 3;
    example = (int*) malloc(sizeof(int) * example_size);
    example[0] = 4;
    example[1] = 5;
    example[2] = 6;

    stack_push(&stack, &example, &example_size);
    stack_pop(&stack, &pop_result, &result_size);

    if(pop_result == example)
        printf("PopResult is the same as the example. Correct\n");
    else {
        printf("PopResult is not the same as the example. Wrong\n");
        *successful_tests = 0;
    }

    printIntValidate("Size of pop_result: %i.", result_size, example_size, successful_tests);
    printIntValidate("Size of stack: %i.", stack.size, 0, successful_tests);

    if(stack.top == NULL)
        printf("Top of stack is null. Correct\n");
    else {
        printf("Top of stack is not null. Wrong\n");
        *successful_tests = 0;
    }
    printf("\n");

    if(example != pop_result)
        free(pop_result);
    free(example);
}

void test_push_and_pop_twice(int* successful_tests) {
    printf("###### Test push two pointers and then pop twice ######\n");
    PointerStack stack;
    int example_size, second_example_size, result_size, second_result_size;
    int *example, *second_example, *pop_result, *pop_second_result;

    stack_init(&stack);
    example_size = 3;
    example = (int*) malloc(sizeof(int) * example_size);
    example[0] = 4;
    example[1] = 5;
    example[2] = 6;

    second_example_size = 4;
    second_example = (int*) malloc(sizeof(int) * second_example_size);
    second_example[0] = 11;
    second_example[1] = 12;
    second_example[2] = 13;
    second_example[3] = 14;

    stack_push(&stack, &example, &example_size);
    stack_push(&stack, &second_example, &second_example_size);
    stack_pop(&stack, &pop_result, &result_size);
    stack_pop(&stack, &pop_second_result, &second_result_size);

    if(pop_result == second_example)
        printf("PopResult is the same as the second_example. Correct\n");
    else {
        printf("PopResult is not the same as the second_example. Wrong\n");
        *successful_tests = 0;
    }
    printIntValidate("Size of pop_result: %i.", result_size, second_example_size,
                     successful_tests);

    if(pop_second_result == example)
        printf("PopSecondResult is the same as the example. Correct\n");
    else {
        printf("PopSecondResult is not the same as the example. Wrong\n");
        *successful_tests = 0;
    }
    printIntValidate("Size of pop_second_result: %i.", second_result_size, example_size,
                     successful_tests);

    printIntValidate("Size of stack: %i.", stack.size, 0, successful_tests);
    if(stack.top == NULL)
        printf("Top of stack is null. Correct\n");
    else {
        printf("Top of stack is not null. Wrong\n");
        *successful_tests = 0;
    }

    printf("\n");

    if(example != pop_second_result)
        free(pop_second_result);
    free(example);

    if(second_example != pop_result)
        free(pop_result);
    free(second_example);
}

void test_alter_push_and_pop(int* successful_tests) {
    printf("###### Test alter between push and pops ######\n");
    PointerStack stack;
    int example_size, second_example_size, third_example_size, result_size,
            second_result_size, third_result_size;
    int *example, *second_example, *third_example, *pop_result, *pop_second_result,
            *pop_third_result;

    stack_init(&stack);
    example_size = 3;
    example = (int*) malloc(sizeof(int) * example_size);
    example[0] = 4;
    example[1] = 5;
    example[2] = 6;

    second_example_size = 4;
    second_example = (int*) malloc(sizeof(int) * second_example_size);
    second_example[0] = 11;
    second_example[1] = 12;
    second_example[2] = 13;
    second_example[3] = 14;

    third_example_size = 2;
    third_example = (int*) malloc(sizeof(int) * third_example_size);
    third_example[0] = 21;
    third_example[1] = 22;

    stack_push(&stack, &example, &example_size);
    stack_push(&stack, &second_example, &second_example_size);
    stack_pop(&stack, &pop_result, &result_size);
    stack_push(&stack, &third_example, &third_example_size);
    stack_pop(&stack, &pop_second_result, &second_result_size);
    stack_pop(&stack, &pop_third_result, &third_result_size);

    if(pop_result == second_example)
        printf("PopResult is the same as the second_example. Correct\n");
    else {
        printf("PopResult is not the same as the second_example. Wrong\n");
        *successful_tests = 0;
    }
    printIntValidate("Size of pop_result: %i.", result_size, second_example_size,
                     successful_tests);

    if(pop_second_result == third_example)
        printf("PopSecondResult is the same as the third_example. Correct\n");
    else {
        printf("PopSecondResult is not the same as the third_example. Wrong\n");
        *successful_tests = 0;
    }
    printIntValidate("Size of pop_second_result: %i.", second_result_size,
                     third_example_size, successful_tests);

    if(pop_third_result == example)
        printf("PopThirdResult is the same as the example. Correct\n");
    else {
        printf("PopThirdResult is not the same as the example. Wrong\n");
        *successful_tests = 0;
    }
    printIntValidate("Size of pop_third_result: %i.", third_result_size, example_size,
                     successful_tests);

    printIntValidate("Size of stack: %i.", stack.size, 0, successful_tests);
    if(stack.top == NULL)
        printf("Top of stack is null. Correct\n");
    else {
        printf("Top of stack is not null. Wrong\n");
        *successful_tests = 0;
    }

    printf("\n");

    if(example != pop_third_result)
        free(pop_third_result);
    free(example);

    if(second_example != pop_result)
        free(pop_result);
    free(second_example);

    if(third_example != pop_second_result)
        free(pop_second_result);
    free(third_example);
}

void test_clear_stack(int* successful_tests) {
    printf("###### Test clear an unempty stack ######\n");
    PointerStack stack;
    int example_size, second_example_size;
    int *example, *second_example;

    stack_init(&stack);
    example_size = 3;
    example = (int*) malloc(sizeof(int) * example_size);
    example[0] = 4;
    example[1] = 5;
    example[2] = 6;

    second_example_size = 4;
    second_example = (int*) malloc(sizeof(int) * second_example_size);
    second_example[0] = 11;
    second_example[1] = 12;
    second_example[2] = 13;
    second_example[3] = 14;

    stack_push(&stack, &example, &example_size);
    stack_push(&stack, &second_example, &second_example_size);
    stack_clear(&stack);

    printIntValidate("Size of stack: %i.", stack.size, 0, successful_tests);

    if(stack.top == NULL)
        printf("Top of stack is null. Correct\n");
    else {
        printf("Top of stack is not null. Wrong\n");
        *successful_tests = 0;
    }
    printf("\n");
}