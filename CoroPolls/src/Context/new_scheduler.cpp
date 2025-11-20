#include <stdlib.h>
#include <stdio.h>
#include "tasklist.h"
#include <ucontext.h>

#if defined(CORO_USE_VALGRIND)
# include <valgrind/valgrind.h>
#endif

const int default_stack_size = 16 * 1024;
const char* memory_alloc_error_msg = "Unable to allocate memory for stack!";
const char* invalid_task_error_msg = "Task argument must be non null!";

struct scheduler_data scheduler_data;
/*
void scheduler_free_current_task(void)
{
	remove_task_tail(&scheduler_data.head, scheduler_data.current, &scheduler_data.tail);
	delete_task_list_item(&scheduler_data.current);
}
*/
struct task* init_task(void (*func)(void *), void *arg)
{
	static int id = 1;
	struct task* task = static_cast<struct task*>(malloc(sizeof(*task)));
	if(!task)
		return NULL;

	task->status = TASK_CREATED;
	task->func = func;
	task->arg = arg;
	task->id = id++;

	return task;
}

void task_call(void)
{
	(*scheduler_data.curr_task)->func(scheduler_data.current->task->arg);
	(*scheduler_data.curr_task)->status = TASK_FINISHED;
//	scheduler_data.current->task->func(scheduler_data.current->task->arg);
// scheduler_data.current->task->status = TASK_FINISHED;
}


void init_task_stack(struct task* task, int stack_size)
{
	task->stack = malloc(stack_size);
    if(!task->stack)
	{
		puts(memory_alloc_error_msg);
        exit(EXIT_FAILURE);
	}

	if (getcontext(&task->continuation) == -1)
		handle_context_error("getcontext");

	task->continuation.uc_stack.ss_sp = task->stack;
    task->continuation.uc_stack.ss_size = stack_size;
  	task->continuation.uc_link = &scheduler_data.continuation;

	makecontext(&task->continuation, (void (*)()) task_call, 0);
}

void create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
	
	init_task_stack(task, default_stack_size);
	
	//struct task_list_item *list_item = init_list_item(task);
	//insert_task_list_item_tail(&scheduler_data.head, list_item, &scheduler_data.tail);
	scheduler_data.task_list.push_back(task);
}


void init_scheduler(void)
{
	scheduler_data.current = scheduler_data.head = scheduler_data.tail = NULL;
	scheduler_data.curr_task = scheduler_data.task_list.end();
}

void task_yield(void)
{
	// if (swapcontext(&scheduler_data.current->task->continuation, &scheduler_data.continuation) == -1)
   	// handle_context_error("swapcontext");
	if (swapcontext(&(*scheduler_data.curr_task)->continuation, &scheduler_data.continuation) == -1)
   		handle_context_error("swapcontext");
}

void delete_task_list_item()
{
	free((*scheduler_data.curr_task)->stack);
	free(*scheduler_data.curr_task);
}

void run_tasks(void)
{
	while(scheduler_data.task_list.size() > 0)
	{
		for(scheduler_data.curr_task = scheduler_data.task_list.begin();
			scheduler_data.curr_task != scheduler_data.task_list.end();
		   )
		   {
				if(((*scheduler_data.curr_task)->status != TASK_RUNNING) && ((*scheduler_data.curr_task)->status != TASK_CREATED))
				{
					scheduler_data.curr_task++;
					continue;
				}				

				if(swapcontext(&scheduler_data.continuation, &(*scheduler_data.curr_task)->continuation) == -1)
            		handle_context_error("swapcontext");

				if((*scheduler_data.curr_task)->status == TASK_FINISHED)
				{
					//delete_task_list_item()
					//free(scheduler_data.curr_task);
					scheduler_data.curr_task = scheduler_data.task_list.erase(scheduler_data.curr_task);
				}
				else
					scheduler_data.curr_task++;
		   }
		
	}
/*	
	struct task *next = NULL;
	while(next = choose_task())
	{
		if (swapcontext(&scheduler_data.continuation, &next->continuation) == -1)
            handle_context_error("swapcontext");
		
		if(next->status == TASK_FINISHED)
			scheduler_free_current_task();
	}*/
	printf("Finished run_tasks!\n");
}