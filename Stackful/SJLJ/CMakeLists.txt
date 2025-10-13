# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)
# Имя вашего проекта
project(MyProject VERSION 1.0 LANGUAGES C)
# Включаем каталог с заголовочными файлами
include_directories(include)

# Подключаем остальные файлы (.c) из каталога src
#target_sources(myapp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

# Добавляем исполняемый файл myapp с точкой входа main.c
add_executable(myapp main.c ${SOURCES})

# Опционально: добавляем флаги компилятора (-Wall, -Wextra etc.)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
