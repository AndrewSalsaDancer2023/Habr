#include <stdlib.h>
#include <stdio.h>
#include "taskscheduler.h"

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
	
	tsk1.num_iters = 3;
	tsk2.num_iters = 7;

	init_scheduler();

	create_task(fibonacci, (void*)&tsk1);
	create_task(fibonacci, (void*)&tsk2);

	run_tasks();
	printf("Finished running all tasks!\n");
	return EXIT_SUCCESS;
}

/*
mkdir build_debug
cd build_debug

cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake --build .
*/