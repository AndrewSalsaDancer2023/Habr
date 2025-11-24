# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)
# Имя вашего проекта
project(MyProject VERSION 1.0 LANGUAGES CXX C)
enable_language(ASM)
# Включаем каталог с заголовочными файлами
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include)

#set(CORO_USE_VALGRIND ON)

set(USE_CONTEXT ON)
#set(USE_ASMCONTEXT ON)
configure_file(config.h.in config.h)


file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Socket/address.cpp)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Socket/basesocket.cpp)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Socket/io_util.cpp)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Socket/epoller.cpp)
set(ASM_SOURCE_FILES)

if(DEFINED USE_CONTEXT)
    list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Context/scheduler.cpp)
else()
   message(FATAL_ERROR "Не задани ни одна из опций USE_SETJMP, USE_CONTEXT. Заверщение сборки")
endif()


# Добавляем исполняемый файл myapp с точкой входа main.c
add_executable(myapp main.cpp ${SOURCES} ${ASM_SOURCE_FILES})

# Опционально: добавляем флаги компилятора (-Wall, -Wextra etc.)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
