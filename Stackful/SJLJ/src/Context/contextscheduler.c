#include <stdlib.h>
#include <stdio.h>

#include "tasklist.h"
#include "contextscheduler.h"
#include <ucontext.h>

#define handle_error(msg) \
         do { perror(msg); exit(EXIT_FAILURE); } while (0)


struct scheduler_data
{
    ucontext_t ctx_scheduler;
	ucontext_t ctx_coro_exit;

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
	scheduler_data.current = NULL;
}

void scheduler_init(void)
{
 	if (getcontext(&scheduler_data.ctx_coro_exit) == -1)
		handle_error("getcontext");

 	int size = 1024;
	void* stack = malloc(size);
    if(!stack)
        return;

    scheduler_data.ctx_coro_exit.uc_stack.ss_sp = stack;
    scheduler_data.ctx_coro_exit.uc_stack.ss_size = size;
    scheduler_data.ctx_coro_exit.uc_link = &scheduler_data.ctx_scheduler;
    makecontext(&scheduler_data.ctx_coro_exit, scheduler_free_current_task, 0);
}

void scheduler_destroy(void)
{
	free(scheduler_data.ctx_coro_exit.uc_stack.ss_sp);
}

void scheduler_create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
	
	//init_task_stack(task);
    
    int size = 16 * 1024;
	task->stack = malloc(size);
    if(!task->stack)
        return;

	if (getcontext(&task->continuation.ctx_func) == -1)
		handle_error("getcontext");

	task->continuation.ctx_func.uc_stack.ss_sp = task->stack;
    task->continuation.ctx_func.uc_stack.ss_size = size;
    task->continuation.ctx_func.uc_link = &scheduler_data.ctx_coro_exit;
    makecontext(&task->continuation.ctx_func, (void (*)()) task->func, 1, task->arg);


	struct task_list *list_item = init_list_item(task);
	
	//sc_list_insert_end(&priv.task_list, &task->task_list);
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
	if (swapcontext(&scheduler_data.current->task->continuation.ctx_func, &scheduler_data.ctx_scheduler) == -1)
            handle_error("swapcontext");
}

void scheduler_run(void)
{
	struct task *next = NULL;

	while(next = scheduler_choose_task())
	{
		if (swapcontext(&scheduler_data.ctx_scheduler, &next->continuation.ctx_func) == -1)
            handle_error("swapcontext");
	}
	printf("Finished scheduler_run!\n");
	//scheduler_destroy();
}