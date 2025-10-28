#pragma once
#include <stdint.h>
// Определяем my_jmp_buf.
// Нам нужно сохранить 8 64-битных регистров + указатель на код.
// rbx, rsp, rbp, r12, r13, r14, r15, а также адрес возврата.
// Индекс 0: rsp
// Индекс 1: rbp
// Индекс 2: rbx
// Индекс 3: r12
// Индекс 4: r13
// Индекс 5: r14
// Индекс 6: r15
// Индекс 7: адрес возврата
typedef uint64_t my_jmp_buf[8];

extern int my_setjmp(my_jmp_buf env);
extern void my_longjmp(my_jmp_buf env, int val);
