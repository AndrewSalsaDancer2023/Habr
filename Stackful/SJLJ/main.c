#include <stdlib.h>
#include <stdio.h>

//#ifdef USE_SETJMP
#include "taskscheduler.h"
/*
#else
	#include "contextscheduler.h"
#endif
*/
/*	
struct tester_args {
	char *name;
	int iters;
};

void tester(void *arg)
{
	int i;
	struct tester_args *ta = (struct tester_args *)arg;
	for (i = 0; i < ta->iters; i++) {
		printf("task %s: %d\n", ta->name, i);
		task_yield();
	}
	free(ta);
}

void create_test_task(char *name, int iters)
{
	struct tester_args *ta = malloc(sizeof(*ta));
	ta->name = name;
	ta->iters = iters;
	scheduler_create_task(tester, ta);
}

int main(int argc, char **argv)
{
	scheduler_init();
	create_test_task("first", 5);
	create_test_task("second", 2);
	scheduler_run();
	printf("Finished running all tasks!\n");
	return EXIT_SUCCESS;
}
*/
/*
struct task 
{
	unsigned long a;
	unsigned long b;	
	unsigned long tmp;
	unsigned long num_iters;
	unsigned long cur_iter;
	unsigned long  task_id;
};
*/
struct task 
{
	unsigned  num_iters;
	unsigned  task_id;
};

void fibonacci(void* arg)
{
	unsigned long  Fn, Fn_2 = 0, Fn_1 = 1;
	int i = 0;

	struct task *tsk = (struct task *)arg;
	for (i = 0; i < tsk->num_iters; i++) 
	{
		if(i == 0)
			Fn = Fn_2;
		else
			if(i == 1) 
				Fn = Fn_1;
			else
			{
				Fn = Fn_2+Fn_1;
				Fn_2 = Fn_1;
				Fn_1 = Fn;
			}
		printf("task %d: %d-th Fibonacci number is: %ld\n", tsk->task_id, i+1, Fn);
		task_yield();
	}
}

struct task tsk1, tsk2;

int main(int argc, char **argv)
{
	tsk1.task_id = 1;
	tsk2.task_id = 2;
	
	tsk1.num_iters = 2;
	tsk2.num_iters = 7;

	scheduler_init();

	scheduler_create_task(fibonacci, (void*)&tsk1);
	scheduler_create_task(fibonacci, (void*)&tsk2);

	scheduler_run();
	printf("Finished running all tasks!\n");
	return EXIT_SUCCESS;
}

/*
mkdir build_debug
cd build_debug

cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake --build .
*/