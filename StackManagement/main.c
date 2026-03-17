#include <stdlib.h>
#include <stdio.h>
#include "taskscheduler.h"
#include "stat.h"
#include "math.h"

const float stack_usage_default = 0.0f;
const int default_stack_size = 16 * 1024;
const uint32_t default_pattern = 0xDEADBEEF;

struct task_params 
{
	unsigned  num_iters;
	unsigned  task_id;
};

struct task_params init_task_params(unsigned  task_id, unsigned  num_iters)
{
	struct task_params params = {.task_id = task_id, .num_iters = num_iters};
	return params;
}

unsigned long fibonacci_recurs(struct task_params *tsk, unsigned int number)
{
	unsigned long  Fn = 0;
	if((number == 0) || (number == 1))
		Fn = number;
	else
		Fn = fibonacci_recurs(tsk, number - 1) + fibonacci_recurs(tsk, number - 2);

	printf("task %d: %d-th Fibonacci number is: %ld\n", tsk->task_id, number+1, Fn);
	task_yield();
	return Fn;
}

void fibonacci(void* arg)
{
	unsigned long  Fn, Fn_2 = 0, Fn_1 = 1;
	int i = 0;

	struct task_params *tsk = (struct task_params *)arg;
	fibonacci_recurs(tsk, tsk->num_iters);
/*
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
	}*/
}

struct task_params tasks[2];
struct coro_statistic task_stat[2];
struct stack_context stacks[2];
// const struct stack_context* stack, struct coro_statistic* stat
void get_statatistic(unsigned int coro_id)
{
	unsigned int length = sizeof(stacks) / sizeof(stacks[0]);
	if(coro_id >= length)
	{
		printf("task %d error: invalid coro_id\n", coro_id);
		return;
	}
	float res = get_stack_usage_percent(&stacks[coro_id], default_pattern);
	task_stat[coro_id].max_stack_usage = fmax(task_stat[coro_id].max_stack_usage, res);
	printf("task %d: maximum stack usage is: %f \n", coro_id, task_stat[coro_id].max_stack_usage);
}

int main(int argc, char **argv)
{
	tasks[0] = init_task_params(0, 3);
	tasks[1] = init_task_params(1, 7);

	task_stat[0] = init_statistick(tasks[0].task_id, stack_usage_default); 
	task_stat[1] = init_statistick(tasks[1].task_id, stack_usage_default);

	stacks[0] = allocate_fixedsize_stack(default_stack_size, 0xDEADBEEF);
	stacks[1] = allocate_fixedsize_stack(default_stack_size, 0xDEADBEEF);

	init_scheduler();

	create_task(fibonacci, (void*)&tasks[0], stacks[0]);
	create_task(fibonacci, (void*)&tasks[1], stacks[1]);

	run_tasks(get_statatistic);
	return EXIT_SUCCESS;
}
