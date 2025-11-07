#include <stdlib.h>
#include <stdio.h>
#include "tasklist.h"

#if defined(CORO_USE_VALGRIND)
# include <valgrind/valgrind.h>
#endif

const int long_jmp_value = 1;

void init_scheduler(void)
{
	scheduler_data.current = scheduler_data.head = scheduler_data.tail = NULL;
}

void init_task_stack(struct task* task, int stack_size)
{
	if(!task)
	{
		puts(invalid_task_error_msg);
        exit(EXIT_FAILURE);
	}

	task->stack = malloc(stack_size);
	task->stack_top = task->stack + stack_size;

#if defined(CORO_USE_VALGRIND)
    task->valgrind_id = VALGRIND_STACK_REGISTER (task->stack , task->stack_top);
#endif
}

void task_yield(void)
{
	if (my_setjmp(scheduler_data.current->task->continuation)) 
		return;
	else 
		my_longjmp(scheduler_data.continuation, long_jmp_value);
}

void scheduler_free_current_task(void)
{
	delete_task_list_item(&scheduler_data.current);
	scheduler_data.current = NULL;
}

void schedule_task(struct task *next)
{
	if (!next) 
	{
		printf("Invalid task  in scheduler! \n");
		return;
	}

	if (next->status == TASK_CREATED) 
	{
		register void *top = next->stack_top;
		__asm__ volatile(
			"movq %[rs], %%rsp \n"
			: [ rs ] "+r" (top) ::
		);

		next->status = TASK_RUNNING;
		next->func(next->arg);
		next->status = TASK_FINISHED;

		my_longjmp(scheduler_data.continuation, long_jmp_value);
	} 
	else
		if (next->status == TASK_RUNNING) 
			my_longjmp(next->continuation, long_jmp_value);
}

void run_tasks(void)
{
	my_setjmp(scheduler_data.continuation);
	struct task *curr = NULL;

	if(scheduler_data.current && (scheduler_data.current->task->status == TASK_FINISHED))
	{
		remove_task_tail(&scheduler_data.head,scheduler_data.current, &scheduler_data.tail);
		scheduler_free_current_task();
	}
	while(curr = choose_task())
	{
		schedule_task(curr);
	}
	printf("Finished run_tasks!\n");
}