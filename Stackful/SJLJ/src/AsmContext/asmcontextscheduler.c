#include <stdlib.h>
#include <stdio.h>

#include "tasklist.h"
#include "contextscheduler.h"

void delete_current_task(void);

struct scheduler_data
{
	struct asm_context_continuation asm_scheduler;

 	struct task_list *head; 	
 	struct task_list *current;
 	struct task_list *tail;

} scheduler_data;

exterb void contextswitch(struct asm_context_continuation *old, struct asm_context_continuation *new);

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
	scheduler_data.current = NULL;
}

void scheduler_init(void)
{
}

static void task_call()
{
	scheduler_data.current->task->func(scheduler_data.current->task->arg);
	scheduler_data.current->task->status = ST_FINISHED;
}

void scheduler_create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
    
    int size = 16 * 1024;
	task->stack = malloc(size);
    if(!task->stack)
        return;
/*
	if (getcontext(&task->continuation.ctx_func) == -1)
		handle_error("getcontext");

	task->continuation.ctx_func.uc_stack.ss_sp = task->stack;
    task->continuation.ctx_func.uc_stack.ss_size = size;
  	task->continuation.ctx_func.uc_link = &scheduler_data.ctx_scheduler.ctx_func;

	makecontext(&task->continuation.ctx_func, (void (*)()) task_call, 1, task);
*/

	*(uint64_t *)&task->stack[size -  8] = (uint64_t)delete_current_task;
	*(uint64_t *)&task->stack[size - 16] = (uint64_t)task_call;
	task->continuation.rsp = (uint64_t)&task->stack[size - 16];


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
//	if (swapcontext(&scheduler_data.current->task->continuation.ctx_func, &scheduler_data.ctx_scheduler.ctx_func) == -1)
//            handle_error("swapcontext");
	contextswitch(&scheduler_data.current->task->continuation, &scheduler_data.asm_scheduler);
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
		//if (swapcontext(&scheduler_data.ctx_scheduler.ctx_func, &next->continuation.ctx_func) == -1)
        //    handle_error("swapcontext");
		contextswitch(&scheduler_data.asm_scheduler, &next->continuation);		
		
		// if(next->status == ST_FINISHED)
		// scheduler_free_current_task();
	}
	printf("Finished scheduler_run!\n");
}