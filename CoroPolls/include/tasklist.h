#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "contextcontinuation.h"
#include "asmcontextcontinuation.h"
#include "config.h"
#include <list>
#include "coroutine.h"

extern const int default_stack_size;
extern const char* memory_alloc_error_msg;
extern const char* invalid_task_error_msg;

#define handle_context_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

/*typedef void (*task_func)(void*);

struct task 
{
	enum task_status status;

	int id;
#if defined(USE_CONTEXT)
	context_continuation continuation;
#elif defined(USE_ASMCONTEXT)
	struct asm_context_continuation continuation;
#endif
	void *stack;
 	task_func func;
	void *arg;
};

struct task_list_item 
{
	struct task_list_item *next;
	struct task *task;
	struct task_list_item *prev;
};
*/

struct scheduler_data
{
	std::list<struct task > task_list;
	std::list<struct task >::iterator curr_task;
};

extern struct scheduler_data scheduler_data;

void insert_task_list_item_tail(struct task_list_item** head, struct task_list_item* task_item, struct task_list_item** tail);
struct task_list_item* remove_task_list_item_head(struct task_list_item** head, struct task_list_item** tail);
void remove_task_tail(struct task_list_item** head, struct task_list_item* item, struct task_list_item** tail);
void delete_task_list_item(struct task_list_item** item);

//struct task_list_item * init_list_item(struct task *task);
//struct task* init_task(void (*func)(void *), void *arg);
//void create_task(void (*func)(void *), void *arg);
extern "C" void create_task(std::function<void (task &)> func);
extern "C" void init_scheduler(void);
extern "C" void run_tasks(void);
//struct task *choose_task(void);
