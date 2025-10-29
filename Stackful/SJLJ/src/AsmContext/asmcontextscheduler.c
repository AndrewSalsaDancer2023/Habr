#include <stdlib.h>
#include <stdio.h>
#include "tasklist.h"

extern void contextswitch(struct asm_context_continuation *old, struct asm_context_continuation *new);

void exit_with_message(const char* message)
{
	puts(message);
	exit(EXIT_FAILURE);
}

void scheduler_free_current_task(void)
{
	remove_task_tail(&scheduler_data.head, scheduler_data.current, &scheduler_data.tail);
	delete_task_list_item(&scheduler_data.current);
}

void init_scheduler(void)
{
	scheduler_data.current = scheduler_data.head = scheduler_data.tail = NULL;
}

static void task_call()
{
	scheduler_data.current->task->func(scheduler_data.current->task->arg);
	scheduler_data.current->task->status = TASK_FINISHED;
}

void task_yield(void)
{
	contextswitch(&scheduler_data.current->task->continuation, &scheduler_data.continuation);
}

void init_task_stack(struct task* task, int stack_size)
{
	task->stack = malloc(stack_size);
    if(!task->stack)
        exit_with_message(memory_alloc_error_msg);

	*(uint64_t *)(task->stack + default_stack_size -  8) = (uint64_t)task_yield;
	*(uint64_t *)(task->stack + default_stack_size - 16) = (uint64_t)task_call;
	task->continuation.rsp = (uint64_t)(task->stack + default_stack_size - 16);
}

void run_tasks(void)
{
	struct task *next = NULL;

	while(next = choose_task())
	{
		contextswitch(&scheduler_data.continuation, &next->continuation);
		if(scheduler_data.current->task->status == TASK_FINISHED)
			scheduler_free_current_task();
	}

	printf("Finished run_tasks!\n");
}