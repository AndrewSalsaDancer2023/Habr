#define _GNU_SOURCE
#include <sys/mman.h>

#include "stackcontext.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"

struct stack_context init_stack(size_t stack_size, void *stack_top) 
{
    // Инициализация через литерал структуры (C99+)
    struct stack_context ctx = { .size = stack_size, .top = stack_top };
    return ctx;
}

void fill_buffer_uint32(void *vp, size_t size, uint32_t value) 
{
    uint32_t *p = (uint32_t *)vp;
    size_t count = size / sizeof(uint32_t); // Сколько 32-битных чисел влезет в буфер

    for (size_t i = 0; i < count; i++) {
        p[i] = value;
    }
}

void fill_buffer_uint8(void *vp, size_t size, uint8_t value) 
{
    uint8_t *p = (uint8_t *)vp;
    size_t count = size / sizeof(uint8_t); // Сколько 32-битных чисел влезет в буфер

    for (size_t i = 0; i < count; i++) {
        p[i] = value;
    }
}

/*
void fillBuffer(void *vp, size_t size, uint32_t value)
{
    uint32_t* start = (uint32_t*)vp;
    // Вычисляем конечное положение (количество элементов = size / sizeof(uint32_t))
    uint32_t* end = start + (size / sizeof(uint32_t));

    std::fill(start, end, value);
}

void fillBufferEx(void *vp, size_t size, uint8_t value)
{
    uint8_t* start = (uint8_t*)vp;
    uint8_t* end = start + (size / sizeof(uint8_t));

    std::fill(start, end, value);
}
*/
struct stack_context allocate_fixedsize_stack(size_t size, uint32_t value)
{
    while(size % sizeof(value))
        size++;
    
    void *vp = malloc(size);
    if (!vp)
    {
        perror("bad alloc"); 
    }
    // Приводим void* к указателю на uint32_t
    // uint32_t* start = static_cast<uint32_t*>(vp);
    // Вычисляем конечное положение (количество элементов = size / sizeof(uint32_t))
    // uint32_t* end = start + (size / sizeof(uint32_t));

    // fillBuffer(vp, size, value);
    fill_buffer_uint8(vp, size, 0xAA);

    // return stack_context(size, static_cast<char*>(vp) + size);
    return init_stack(size, vp);
}

void deallocate_fixedsize_stack(struct stack_context* sctx)
{
    // void *vp = static_cast<char*>(sctx.top) - sctx.size;
    void *vp = sctx->top;
    free(vp);
}

struct stack_context allocate_protected_stack(size_t size, uint32_t value)
{
        errno = 0;
        long page_size = get_page_size();

        if(page_size == -1)
        {
            if (errno != 0)
                perror("impossible to get page size");
            else
                fprintf(stderr, "get page size is onot supported\n");
        }
//            throw std::logic_error("impossible to get page size");
        // calculate how many pages are required
        // const std::size_t pages = (size_ + page_size - 1) / page_size;
        const size_t pages = (size + page_size - 1) / page_size;
        // add one page at bottom that will be used as guard-page
        const size_t size__ = (pages + 1) * page_size;

        void* vp = mmap(0, size__, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

        if (MAP_FAILED == vp) 
            perror("mmap failed"); 

        fill_buffer_uint32(vp, size__, value);
        // conforming to POSIX.1-2001
        const int result = mprotect(vp, page_size, PROT_NONE);
        if(result == -1)
            perror("mprotect failed"); 

        // return stack_context(size, static_cast<char*>(vp) + size);
        return init_stack(size, vp);
}

void deallocate_protected_stack(struct stack_context* sctx)
{
    if(!sctx->top)
        fprintf(stderr, "sctx.sp schoudn't be null\n");
        // throw std::logic_error("sctx.sp schoudn't be null");

    void * vp = ((char*)sctx->top) - sctx->size;
    // conform to POSIX.4 (POSIX.1b-1993, _POSIX_C_SOURCE=199309L)
    munmap(vp, sctx->size);
}
