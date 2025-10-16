#include <stdlib.h>
#include <stdio.h>

#include "tasklist.h"

enum 
{
	TASK_INIT=0,
	TASK_SCHEDULE,
	TASK_EXIT,
};


struct scheduler_data
{
	jmp_buf buf;

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

	task->stack_size = 16 * 1024;
	task->stack_bottom = malloc(task->stack_size);
	task->stack_top = task->stack_bottom + task->stack_size;
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
	
	//sc_list_insert_end(&priv.task_list, &task->task_list);
	insert_task_list_tail(&scheduler_data.head, list_item, &scheduler_data.tail);
}

void scheduler_exit_current_task(void)
{
	//struct task *task = priv.current->task;
	/* Remove so we don't schedule this again */
//	sc_list_remove(&task->task_list);
 	remove_task_tail(&scheduler_data.head,scheduler_data.current, &scheduler_data.tail);
	/* Would love to free the task... but if we do, we won't have a
	 * stack anymore, which would really put a damper on things.
	 * Let's defer that until we longjmp back into the old stack */
	longjmp(scheduler_data.buf, TASK_EXIT);
	/* NO RETURN */
}

static struct task *scheduler_choose_task(void)
{
	// struct task_list* item = priv.head;
	scheduler_data.current = scheduler_data.head;
	while(scheduler_data.current)
	{
		if((scheduler_data.current->task->status == ST_RUNNING) || (scheduler_data.current->task->status == ST_CREATED)) 
		{
			scheduler_data.current = remove_task_list_head(&scheduler_data.head, &scheduler_data.tail);
			insert_task_list_tail(&scheduler_data.head, scheduler_data.current, &scheduler_data.tail);
			// priv.current = item;
			return scheduler_data.current->task;
		}
	}
	return NULL;
}

static void schedule(void)
{

	struct task *next = scheduler_choose_task();

	if (!next) {
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


		scheduler_exit_current_task();
	} else {
		longjmp(next->continuation.buf, TASK_SCHEDULE);
	}

}

void task_yield(void)
{
	if (setjmp(scheduler_data.current->task->continuation.buf)) {
		return;
	} else {
		longjmp(scheduler_data.buf, TASK_SCHEDULE);
	}
}

static void scheduler_free_current_task(void)
{
	delete_task_list(&scheduler_data.current);
	scheduler_data.current = NULL;
}


void scheduler_run(void)
{
	switch (setjmp(scheduler_data.buf)) {
	case TASK_EXIT:
		scheduler_free_current_task();
	case TASK_INIT:
	case TASK_SCHEDULE:
		schedule();
		return;
	default:
		fprintf(stderr, "Scheduler error! \n");
		return;
	}
	
}