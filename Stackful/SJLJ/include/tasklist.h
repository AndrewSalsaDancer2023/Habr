#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "sjlj_continuation.h"
#include "contextcontinuation.h"
#include "asmcontextcontinuation.h"
#include "config.h"

enum task_status {
	ST_CREATED,
	ST_RUNNING,
	ST_WAITING,
	ST_FINISHED,
};

typedef void (*task_func)(void*);

struct task {
	enum task_status status;

	int id;
	/*
	 * For tasks in the ST_RUNNING state, this is where to longjmp back to
	 * in order to resume their execution.
	 */
	//jmp_buf buf;
	#if defined(USE_SETJMP)
		struct sjlj_continuation continuation;
		void *stack_top;
		void *stack_bottom;
		int stack_size;
	#elif defined(USE_CONTEXT)
		struct context_continuation continuation;
		void *stack;
	#elif defined(USE_ASMCONTEXT)
		struct asm_context_continuation continuation;
		char *stack;
	#endif
	/*
	 * Function and argument to call on startup.
	 */
 	task_func func;
	void *arg;
};


struct task_list {
	struct task_list *next;
	struct task *task;
	struct task_list *prev;
};

void insert_task_list_tail(struct task_list** head, struct task_list* task_item, struct task_list** tail);
struct task_list* remove_task_list_head(struct task_list** head, struct task_list** tail);
void remove_task_tail(struct task_list** head, struct task_list* item, struct task_list** tail);

struct task_list * init_list_item(struct task *task);
struct task* init_task(void (*func)(void *), void *arg);
//void init_task_stack(struct task* task);
//void delete_task_list(struct task_list** item);
