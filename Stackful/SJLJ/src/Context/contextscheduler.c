#include <stdlib.h>
#include <stdio.h>

#include "tasklist.h"
#include "contextscheduler.h"
#include <ucontext.h>

#define handle_error(msg) \
         do { perror(msg); exit(EXIT_FAILURE); } while (0)

const int default_stack_size = 16 * 1024;

struct scheduler_data
{
	context_continuation scheduler_continuation;

 	struct task_list *head; 	
 	struct task_list *current;
 	struct task_list *tail;

} scheduler_data;


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

static void task_call(void)
{
	scheduler_data.current->task->func(scheduler_data.current->task->arg);
	scheduler_data.current->task->status = ST_FINISHED;
}

void init_task_stack(struct task* task, int stack_size)
{
	task->stack = malloc(stack_size);
    if(!task->stack)
        return;

	if (getcontext(&task->continuation) == -1)
		handle_error("getcontext");

	task->continuation.uc_stack.ss_sp = task->stack;
    task->continuation.uc_stack.ss_size = stack_size;
  	task->continuation.uc_link = &scheduler_data.scheduler_continuation;

	makecontext(&task->continuation, (void (*)()) task_call, 0);
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
	if (swapcontext(&scheduler_data.current->task->continuation, &scheduler_data.scheduler_continuation) == -1)
            handle_error("swapcontext");
}

void scheduler_run(void)
{
	struct task *next = NULL;

	while(next = scheduler_choose_task())
	{
		if (swapcontext(&scheduler_data.scheduler_continuation, &next->continuation) == -1)
            handle_error("swapcontext");
		
		if(next->status == ST_FINISHED)
			scheduler_free_current_task();
	}
	printf("Finished scheduler_run!\n");
}