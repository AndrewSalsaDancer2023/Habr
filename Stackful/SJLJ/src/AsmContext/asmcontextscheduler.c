#include <stdlib.h>
#include <stdio.h>

#include "tasklist.h"
#include "contextscheduler.h"

const int default_stack_size = 16 * 1024;
const char* mem_alloc_message = "Invalid memory allocation";

void delete_current_task(void);

struct scheduler_data
{
	struct asm_context_continuation scheduler_continuation;

 	struct task_list *head; 	
 	struct task_list *current;
 	struct task_list *tail;

} scheduler_data;

extern void contextswitch(struct asm_context_continuation *old, struct asm_context_continuation *new);


void exit_with_message(const char* message)
{
	puts(message);
	exit(EXIT_FAILURE);
}

void delete_task_list(struct task_list** item)
{
	free((*item)->task->stack);
	free((*item)->task);
	free(*item);
}

void scheduler_free_current_task(void)
{
	remove_task_tail(&scheduler_data.head, scheduler_data.current, &scheduler_data.tail);
	delete_task_list(&scheduler_data.current);
}

void scheduler_init(void)
{
}

static void task_call()
{
	scheduler_data.current->task->func(scheduler_data.current->task->arg);
	scheduler_data.current->task->status = ST_FINISHED;
}

void init_task_stack(struct task* task, int stack_size)
{
	task->stack = malloc(stack_size);
    if(!task->stack)
        exit_with_message(mem_alloc_message);

	*(uint64_t *)(task->stack + default_stack_size -  8) = (uint64_t)delete_current_task;
	*(uint64_t *)(task->stack + default_stack_size - 16) = (uint64_t)task_call;
	task->continuation.rsp = (uint64_t)(task->stack + default_stack_size - 16);
}

void scheduler_create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
    init_task_stack(task, default_stack_size);

	struct task_list *list_item = init_list_item(task);
	
	insert_task_list_tail(&scheduler_data.head, list_item, &scheduler_data.tail);
}

static struct task *scheduler_choose_task(void)
{
	scheduler_data.current = scheduler_data.head;
	while(scheduler_data.current)
	{
		if((scheduler_data.current->task->status == ST_RUNNING) || (scheduler_data.current->task->status == ST_CREATED)) 
		{
			scheduler_data.current = remove_task_list_head(&scheduler_data.head, &scheduler_data.tail);
			insert_task_list_tail(&scheduler_data.head, scheduler_data.current, &scheduler_data.tail);

			return scheduler_data.current->task;
		}
	}
	return NULL;
}

void task_yield(void)
{
	contextswitch(&scheduler_data.current->task->continuation, &scheduler_data.scheduler_continuation);
}

void delete_current_task(void)
{
	if(scheduler_data.current->task->status == ST_FINISHED)
		scheduler_free_current_task();

	scheduler_run();
}

void scheduler_run(void)
{
	struct task *next = NULL;

	while(next = scheduler_choose_task())
	{
		contextswitch(&scheduler_data.scheduler_continuation, &next->continuation);
	}

	printf("Finished scheduler_run!\n");
}