cmake_minimum_required(VERSION 3.10)
# Имя вашего проекта
project(MyProject VERSION 1.0 LANGUAGES CXX)
include_directories(include)

#set(CORO_USE_VALGRIND ON)


file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/socket/address.cpp)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/socket/basesocket.cpp)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/socket/epoller.cpp)

list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/scheduler/scheduler.cpp)

# Добавляем исполняемый файл myapp с точкой входа main.c
add_executable(myapp main.cpp ${SOURCES})

# Опционально: добавляем флаги компилятора (-Wall, -Wextra etc.)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
