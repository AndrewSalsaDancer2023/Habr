#pragma once
#include <stdint.h>
// We nned to save 8 64 bit egisters.
// rbx, rsp, rbp, r12, r13, r14, r15 and return address.
// index 0: rsp
// index 1: rbp
// index 2: rbx
// index 3: r12
// index 4: r13
// index 5: r14
// index 6: r15
// index 7: return address
typedef uint64_t my_jmp_buf[8];

extern int my_setjmp(my_jmp_buf env);
extern void my_longjmp(my_jmp_buf env, int val);
