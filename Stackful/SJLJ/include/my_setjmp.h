#pragma once
#include <stdint.h>
// Define my_jmp_buf.
// We need to store 8 64-bit registers.
// rbx, rsp, rbp, r12, r13, r14, r15, and return address.
// index 0: rbx
// index 1: rbp
// index 2: r12
// index 3: r13
// index 4: r14
// index 5: r15
// index 6: rsp
// index 7: return address
typedef uint64_t my_jmp_buf[8];

extern int my_setjmp(my_jmp_buf env);
extern void my_longjmp(my_jmp_buf env, int val);
