#include <stdio.h>
#include <stdlib.h>
#include "my_setjmp.h"

int max_iters, i, j;
my_jmp_buf   main_continuation;        
my_jmp_buf   coro1_continuation;
my_jmp_buf   coro2_continuation;    

void      coro1(void);
void      coro2(void);

void  main(int  argc, char* argv[])
{
     max_iters = 5;
     if (my_setjmp(main_continuation) == 0)
          Coro1();
     if (my_setjmp(main_continuation) == 0)
          Coro2();
     my_longjmp(coro1_continuation, 1);
}

void  coro1(void)
{
     if (my_setjmp(coro1_continuation) == 0) {
          i = 1;
          my_longjmp(main_continuation, 1);
     }
     while (1) {
          printf("Coro1 i: (%d), addr i: %p - ", i, &i);
          i++;
          if (my_setjmp(coro1_continuation) == 0)
               my_longjmp(coro2_continuation, 1);
     }
}

void  coro2(void)
{
     if (my_setjmp(coro2_continuation) == 0) {
          j = 1;
          my_longjmp(main_continuation, 1);
     }
     while (1) {
          printf("Coro2 j:(%d), addr j: %p\n", j, &j);
          j++;
          if (j > max_iters)
               exit(0);
          if (my_setjmp(coro2_continuation) == 0)
               my_longjmp(coro1_continuation, 1);
     }
}
