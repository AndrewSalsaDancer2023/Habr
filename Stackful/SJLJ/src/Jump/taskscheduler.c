#include <stdlib.h>
#include <stdio.h>

#include "tasklist.h"

enum 
{
	TASK_INIT=0,
	TASK_SCHEDULE,
	TASK_EXIT,
};

const int default_stack_size = 16 * 1024;

struct scheduler_data
{
	sjlj_continuation continuation;

 	struct task_list *head; 	
 	struct task_list *current;
 	struct task_list *tail;

} scheduler_data;

void scheduler_init(void)
{
	scheduler_data.current = scheduler_data.head = scheduler_data.tail = NULL;
}


void init_task_stack(struct task* task)
{
	if(!task)
		return;

	task->stack_bottom = malloc(default_stack_size);
	task->stack_top = task->stack_bottom + default_stack_size;
}

void delete_task_list(struct task_list** item)
{
	free((*item)->task->stack_bottom);
	free((*item)->task);
	free(*item);
}

void scheduler_create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
	
	init_task_stack(task);
	
	struct task_list *list_item = init_list_item(task);
	insert_task_list_tail(&scheduler_data.head, list_item, &scheduler_data.tail);
}

void scheduler_exit_current_task(void)
{
	/* Would love to free the task... but if we do, we won't have a
	 * stack anymore, which would really put a damper on things.
	 * Let's defer that until we longjmp back into the old stack */
	longjmp(scheduler_data.continuation, TASK_EXIT);
	/* NO RETURN */
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
	int jmpres = setjmp(scheduler_data.current->task->continuation);
	if (jmpres) {
		return;
	} else {
		longjmp(scheduler_data.continuation, TASK_SCHEDULE);
	}
}

static void scheduler_free_current_task(void)
{
	delete_task_list(&scheduler_data.current);
	scheduler_data.current = NULL;
}


void schedule_task(struct task *next)
{
	if (!next) {
		fprintf(stderr, "Invalid null task  in scheduler! \n");
		return;
	}

	if (next->status == ST_CREATED) {

		register void *top = next->stack_top;
		__asm__ volatile(
			"mov %[rs], %%rsp \n"
			: [ rs ] "+r" (top) ::
		);

		next->status = ST_RUNNING;
		next->func(next->arg);
		next->status = ST_FINISHED;

		scheduler_exit_current_task();
	} else {
		longjmp(next->continuation, TASK_SCHEDULE);
	}
}


void scheduler_run(void)
{
	int task_state = setjmp(scheduler_data.continuation);
	struct task *curr = NULL;

	if(scheduler_data.current && (scheduler_data.current->task->status == ST_FINISHED))
	{
		remove_task_tail(&scheduler_data.head,scheduler_data.current, &scheduler_data.tail);
		scheduler_free_current_task();
	}
	while(curr = scheduler_choose_task())
	{
		schedule_task(curr);
	}
	printf("Finished scheduler_run!\n");
}
