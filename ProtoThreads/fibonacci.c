#include <stdio.h>
#include <stdlib.h>
#include "lib/pt.h"

struct task {
	unsigned long a;
	unsigned long b;	
	unsigned long tmp;
	struct pt state;
};

PT_THREAD(fibonacci(struct task *tsk))
{

  PT_BEGIN(&tsk->state);
  while(1) {
	PT_YIELD(&tsk->state);
	tsk->tmp = tsk->a;
	tsk->a = tsk->b;
	tsk->b = tsk->b + tsk->tmp;
  }
  
  PT_END(&tsk->state);
}
 
int main(void)
{
  int i = 0;
  
  struct task tsk;
  tsk.a = 0;
  tsk.b = 1;  

  PT_INIT(&tsk.state);
  
  while(i < 10) {
    fibonacci(&tsk);
    printf("fibonacci %d : %lu \n", i+1, tsk.a);
    i++;
  }
  return EXIT_SUCCESS;
}

