cmake_minimum_required(VERSION 3.7)
project(Tarea2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c src/input_reader.c include/input_reader.h src/pointer_stack.c include/pointer_stack.h test/test.c test/pointer_stack_test.c test/pointer_stack_test.h)
add_executable(Tarea2 ${SOURCE_FILES})