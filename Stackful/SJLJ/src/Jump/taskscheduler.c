#include <stdlib.h>
#include <stdio.h>

#include "tasklist.h"

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
}

void task_yield(void)
{
	int jmpres = setjmp(scheduler_data.current->task->continuation);
	if (jmpres) {
		return;
	} else {
		longjmp(scheduler_data.continuation, long_jmp_value);
	}
}

static void scheduler_free_current_task(void)
{
	delete_task_list_item(&scheduler_data.current);
	scheduler_data.current = NULL;
}

void schedule_task(struct task *next)
{
	if (!next) 
	{
		fprintf(stderr, "Invalid null task  in scheduler! \n");
		return;
	}

	if (next->status == TASK_CREATED) 
	{
		register void *top = next->stack_top;
		__asm__ volatile(
			"mov %[rs], %%rsp \n"
			: [ rs ] "+r" (top) ::
		);

		next->status = TASK_RUNNING;
		next->func(next->arg);
		next->status = TASK_FINISHED;

		longjmp(scheduler_data.continuation, long_jmp_value);
	} 
	else 
		longjmp(next->continuation, long_jmp_value);
}

void run_tasks(void)
{
	int task_state = setjmp(scheduler_data.continuation);
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