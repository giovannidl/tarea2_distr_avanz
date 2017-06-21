//
// Created by gio on 21-06-17.
//

#include <memory.h>
#include <stdio.h>
#include "../include/test_tools.h"

int checkArg(char** argv, int argc, char* value) {
    if(argc < 2)
        return 0;

    int i;
    for(i = 1; i < argc; i++)
        if(!strcmp(argv[i], value))
            return 1;

    return 0;
}

int printIntValidate(char* message, int answer, int correct_answer, int* successful_tests) {
    printf(message, answer);
    if(answer == correct_answer)
        printf(" Correct\n");
    else {
        printf(" Wrong\n");
        *successful_tests = 0;
    }
}
