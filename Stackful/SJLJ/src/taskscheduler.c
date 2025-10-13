#include <stdlib.h>
#include <stdio.h>

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

void scheduler_create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
	
	init_task_stack(task);
	
	struct task_list *list_item = init_list_item(task);
	
	//sc_list_insert_end(&priv.task_list, &task->task_list);
	scheduler_insert_task_list_tail(&priv.head, list_item, &priv.tail);
}

void scheduler_exit_current_task(void)
{
	//struct task *task = priv.current->task;
	/* Remove so we don't schedule this again */
//	sc_list_remove(&task->task_list);
 	scheduler_remove_task_from_tail(&priv.head,priv.current, &priv.tail);
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
			item = scheduler_remove_task_list_head(&priv.head, &priv.tail);
			scheduler_insert_task_list_tail(&priv.head, item, &priv.tail);
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
		fprintf(stderr, "Scheduler error! \n");
		return;
	}
	
}

