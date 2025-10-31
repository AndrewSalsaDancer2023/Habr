#pragma once
#include <stdint.h>
// Define my_jmp_buf.
// We need to store 8 64-bit registers.
// rbx, rsp, rbp, r12, r13, r14, r15, and return address.
// Индекс 0: rbx
// Индекс 1: rbp
// Индекс 2: r12
// Индекс 3: r13
// Индекс 4: r14
// Индекс 5: r15
// Индекс 6: rsp
// Индекс 7: return address
typedef uint64_t my_jmp_buf[8];

extern int my_setjmp(my_jmp_buf env);
extern void my_longjmp(my_jmp_buf env, int val);
