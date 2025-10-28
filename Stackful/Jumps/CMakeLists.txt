# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

project(MyProject VERSION 1.0 LANGUAGES C)
enable_language(ASM)

# Добавляем исполняемый файл myapp с точкой входа main.c
add_executable(myapp main.c jmp_functions.s)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
