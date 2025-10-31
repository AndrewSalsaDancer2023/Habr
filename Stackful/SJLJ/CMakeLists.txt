# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

project(MyProject VERSION 1.0 LANGUAGES C)
enable_language(ASM)

# Включаем каталог с заголовочными файлами
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include)

set(USE_SETJMP ON)
#set(USE_CONTEXT ON)
#set(USE_ASMCONTEXT ON)
configure_file(config.h.in config.h)


file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

if(DEFINED USE_SETJMP)
   list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Jump/taskscheduler.c)
elseif(DEFINED USE_CONTEXT)
   list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Context/contextscheduler.c)
elseif(DEFINED USE_ASMCONTEXT)
   list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/AsmContext/asmcontextscheduler.c)
   list(APPEND ASM_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Asm/asm_function.s)
else()
   message(FATAL_ERROR "Не задана ни одна из опций USE_SETJMP, USE_CONTEXT, USE_ASMCONTEXT. Заверщение сборки")
endif()


# Добавляем исполняемый файл myapp с точкой входа main.c
add_executable(myapp main.c ${SOURCES} ${ASM_SOURCE_FILES})

# Опционально: добавляем флаги компилятора (-Wall, -Wextra etc.)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
