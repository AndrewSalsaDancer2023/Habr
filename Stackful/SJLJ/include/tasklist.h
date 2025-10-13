#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>

enum task_status {
	ST_CREATED,
	ST_RUNNING,
	ST_WAITING,
};

typedef void (*task_func)(void*);

struct task {
	enum task_status status;

	int id;
	/*
	 * For tasks in the ST_RUNNING state, this is where to longjmp back to
	 * in order to resume their execution.
	 */
	jmp_buf buf;

	/*
	 * Function and argument to call on startup.
	 */
 	task_func func;
	void *arg;


	void *stack_bottom;
	void *stack_top;
	int stack_size;
};


struct task_list {
	struct task_list *next;
	struct task *task;
	struct task_list *prev;
};

void scheduler_insert_task_list_tail(struct task_list** head, struct task_list* task_item, struct task_list** tail);
struct task_list* scheduler_remove_task_list_head(struct task_list** head, struct task_list** tail);
void scheduler_remove_task_from_tail(struct task_list** head, struct task_list* item, struct task_list** tail);

struct task_list * init_list_item(struct task *task);
struct task* init_task(void (*func)(void *), void *arg);
void init_task_stack(struct task* task);
