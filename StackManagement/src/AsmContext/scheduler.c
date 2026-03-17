#include <stdlib.h>
#include <stdio.h>
#include "tasklist.h"

#if defined(CORO_USE_VALGRIND)
# include <valgrind/valgrind.h>
#endif

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

void task_call()
{
	scheduler_data.current->task->func(scheduler_data.current->task->arg);
	scheduler_data.current->task->status = TASK_FINISHED;
}

void task_yield(void)
{
	contextswitch(&scheduler_data.current->task->continuation, &scheduler_data.continuation);
}

void init_task_stack(struct task* task, struct stack_context stack)
{
	// task->stack = malloc(stack_size);
	// task->stack = allocate_fixedsize_stack(stack_size, 0xDEADBEEF);
	task->stack = stack;
    if(!task->stack.top)
        exit_with_message(memory_alloc_error_msg);

#if defined(CORO_USE_VALGRIND)
    task->valgrind_id = VALGRIND_STACK_REGISTER (task->stack, task->stack.top+task->stack.size);
#endif

	*(uint64_t *)(task->stack.top + task->stack.size -  8) = (uint64_t)task_yield;
	*(uint64_t *)(task->stack.top + task->stack.size - 16) = (uint64_t)task_call;
	task->continuation.rsp = (uint64_t)(task->stack.top + task->stack.size - 16);
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