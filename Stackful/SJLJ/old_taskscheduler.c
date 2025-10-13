#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "tasklist.h"

enum {
	INIT=0,
	SCHEDULE,
	EXIT_TASK,
};


struct scheduler_private {
	/*
	 * Where to jump back to perform scheduler actions
	 */
	jmp_buf buf;

	/*
	 * The current task
	 */
 	struct task_list *head;
 	
 	struct task_list *current;
 	struct task_list *tail;

} priv;

void scheduler_init(void)
{
	priv.current = priv.head = priv.tail = NULL;
}

void scheduler_insert_task_list_tail(struct task_list *task_item)
{
	if(!task_item)
		return;
		
	if(!priv.tail) 
	{
		task_item->next = NULL;
		task_item->prev = NULL;
	}
	else
	{
		priv.tail->next = task_item;
		task_item->prev = priv.tail;
		task_item->next = NULL;
	}
	priv.tail = task_item;

	if(!priv.head)
		priv.head = task_item;
}

struct task_list* scheduler_remove_task_list_head(void)
{
	struct task_list *item = NULL;
	if(!priv.head)
		return NULL;

	item = priv.head;	
	if(priv.head == priv.tail) 
	{
		priv.head = priv.tail = NULL;
//		return item;
	}
	else
	{
//	item = priv.head;
		priv.head = priv.head->next;
	}
	return item;
}

void scheduler_remove_task_from_list(struct task_list* item)
{
	if(!item)
		return;
	
	if(priv.head == item)
	{
		priv.head = item->next;
	}
	
	if(priv.tail == item) 
	{
  	priv.tail = item->prev;
	}
	
	item->prev = item->next;
}

struct task_list * init_list_item(struct task *task)
{
	struct task_list *list_item = malloc(sizeof(*list_item));
	if(!list_item)
		return NULL;
	list_item->next = list_item->prev = NULL;
	list_item->task = task;

	return list_item;
}

struct task* init_task(void (*func)(void *), void *arg)
{
	static int id = 1;
	struct task* task = malloc(sizeof(*task));
	if(!task)
		return NULL;

	task->status = ST_CREATED;
	task->func = func;
	task->arg = arg;
	task->id = id++;

	return task;
}

void init_task_stack(struct task* task)
{
	if(!task)
		return;

	task->stack_size = 16 * 1024;
	task->stack_bottom = malloc(task->stack_size);
	task->stack_top = task->stack_bottom + task->stack_size;
}

void scheduler_create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
	
	init_task_stack(task);
	
	struct task_list *list_item = init_list_item(task);
	
	//sc_list_insert_end(&priv.task_list, &task->task_list);
	scheduler_insert_task_list_tail(list_item);
}

void scheduler_exit_current_task(void)
{
	//struct task *task = priv.current->task;
	/* Remove so we don't schedule this again */
//	sc_list_remove(&task->task_list);
 	scheduler_remove_task_from_list(priv.current);
	/* Would love to free the task... but if we do, we won't have a
	 * stack anymore, which would really put a damper on things.
	 * Let's defer that until we longjmp back into the old stack */
	longjmp(priv.buf, EXIT_TASK);
	/* NO RETURN */
}

static struct task *scheduler_choose_task(void)
{
	struct task_list* item = priv.head;
	while(item)
	{
		if((item->task->status == ST_RUNNING) || (item->task->status == ST_CREATED)) 
		{
			item = scheduler_remove_task_list_head();
			scheduler_insert_task_list_tail(item);
			priv.current = item;
			return item->task;
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

	//priv.current = next;
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
		longjmp(next->buf, 1);
	}

}

void scheduler_relinquish(void)
{
	if (setjmp(priv.current->task->buf)) {
		return;
	} else {
		longjmp(priv.buf, SCHEDULE);
	}
}

static void scheduler_free_current_task(void)
{
	free(priv.current->task->stack_bottom);
	free(priv.current->task);
	free(priv.current);
	priv.current = NULL;
}


void scheduler_run(void)
{

	switch (setjmp(priv.buf)) {
	case EXIT_TASK:
		scheduler_free_current_task();
	case INIT:
	case SCHEDULE:
		schedule();
		return;
	default:
		fprintf(stderr, "Uh oh, scheduler error\n");
		return;
	}
	
}

