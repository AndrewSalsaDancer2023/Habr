#include <stdlib.h>
#include <stdio.h>
#include "tasklist.h"
#include <ucontext.h>

#if defined(CORO_USE_VALGRIND)
# include <valgrind/valgrind.h>
#endif

void scheduler_free_current_task(void)
{
	remove_task_tail(&scheduler_data.head, scheduler_data.current, &scheduler_data.tail);
	delete_task_list_item(&scheduler_data.current);
}

void init_scheduler(void)
{
	scheduler_data.current = scheduler_data.head = scheduler_data.tail = NULL;
}

void task_call(void)
{
	scheduler_data.current->task->func(scheduler_data.current->task->arg);
	scheduler_data.current->task->status = TASK_FINISHED;
}

extern "C" void init_task_stack(struct task* task, int stack_size)
{
	task->stack = malloc(stack_size);
    if(!task->stack)
	{
		puts(memory_alloc_error_msg);
        exit(EXIT_FAILURE);
	}

	if (getcontext(&task->continuation) == -1)
		handle_context_error("getcontext");

#if defined(CORO_USE_VALGRIND)
    task->valgrind_id = VALGRIND_STACK_REGISTER (task->stack , task->stack + stack_size);
#endif

	task->continuation.uc_stack.ss_sp = task->stack;
    task->continuation.uc_stack.ss_size = stack_size;
  	task->continuation.uc_link = &scheduler_data.continuation;

	makecontext(&task->continuation, (void (*)()) task_call, 0);
}

void task_yield(void)
{
	if (swapcontext(&scheduler_data.current->task->continuation, &scheduler_data.continuation) == -1)
    	handle_context_error("swapcontext");
}

void run_tasks(void)
{
	struct task *next = NULL;

	while(next = choose_task())
	{
		if (swapcontext(&scheduler_data.continuation, &next->continuation) == -1)
            handle_context_error("swapcontext");
		
		if(next->status == TASK_FINISHED)
			scheduler_free_current_task();
	}
	printf("Finished run_tasks!\n");
}