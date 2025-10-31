#include <stdio.h>
#include <stdlib.h>
#include "my_setjmp.h"

int max_iters, i, j;
my_jmp_buf   MainContinuation;        
my_jmp_buf   Coro1Continuation;
my_jmp_buf   Coro2Continuation;    

void      Coro1(void);
void      Coro2(void);

void  main(int  argc, char* argv[])
{
     max_iters = 5;
     if (my_setjmp(MainContinuation) == 0)
          Coro1();
     if (my_setjmp(MainContinuation) == 0)
          Coro2();
     my_longjmp(Coro1Continuation, 1);
}

void  Coro1(void)
{
     if (my_setjmp(Coro1Continuation) == 0) {
          i = 1;
          my_longjmp(MainContinuation, 1);
     }
     while (1) {
          printf("Coro1 i: (%d), addr i: %p - ", i, &i);
          i++;
          if (my_setjmp(Coro1Continuation) == 0)
               my_longjmp(Coro2Continuation, 1);
     }
}

void  Coro2(void)
{
     if (my_setjmp(Coro2Continuation) == 0) {
          j = 1;
          my_longjmp(MainContinuation, 1);
     }
     while (1) {
          printf("Coro2 j:(%d), addr j: %p\n", j, &j);
          j++;
          if (j > max_iters)
               exit(0);
          if (my_setjmp(Coro2Continuation) == 0)
               my_longjmp(Coro1Continuation, 1);
     }
}
