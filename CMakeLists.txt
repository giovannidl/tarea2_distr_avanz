cmake_minimum_required(VERSION 3.7)
project(Tarea2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c src/input_reader.c include/input_reader.h src/pointer_stack.c include/pointer_stack.h test/test.c test/src/pointer_stack_test.c test/include/pointer_stack_test.h src/minor_tools.c include/minor_tools.h test/src/minor_tools_test.c test/include/minor_tools_test.h test/src/test_tools.c test/include/test_tools.h)
add_executable(Tarea2 ${SOURCE_FILES})