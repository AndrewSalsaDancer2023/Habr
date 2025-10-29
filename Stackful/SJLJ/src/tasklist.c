#include "tasklist.h"
#include <stdlib.h>

const int default_stack_size = 16 * 1024;
const char* memory_alloc_error_msg = "Unable to allocate memory for stack!";
const char* invalid_task_error_msg = "Task argument must be non null!";

struct scheduler_data scheduler_data;
void init_task_stack(struct task* task, int stack_size);

struct task_list_item * init_list_item(struct task *task)
{
	struct task_list_item *list_item = malloc(sizeof(*list_item));
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

	task->status = TASK_CREATED;
	task->func = func;
	task->arg = arg;
	task->id = id++;

	return task;
}

void create_task(void (*func)(void *), void *arg)
{
	struct task* task =init_task(func, arg);
	
	init_task_stack(task, default_stack_size);
	
	struct task_list_item *list_item = init_list_item(task);
	insert_task_list_item_tail(&scheduler_data.head, list_item, &scheduler_data.tail);
}

void insert_task_list_item_tail(struct task_list_item** head, struct task_list_item* task_item, struct task_list_item** tail)
{
	if(!task_item)
		return;
		
	if(*tail)
	{
		(*tail)->next = task_item;
		task_item->prev = *tail;
		task_item->next = NULL;
	}
	else
	{
		task_item->next = NULL;		
		task_item->prev = NULL;
	}
	*tail = task_item;

	if(!(*head))
		*head = task_item;
}

struct task_list_item* remove_task_list_item_head(struct task_list_item** head, struct task_list_item** tail)
{
	struct task_list_item *item = NULL;
	if(!(*head))
		return NULL;

	item = *head;	
	if((*head) == (*tail))
		*head = *tail = NULL;
	else
		*head = (*head)->next;

	return item;
}

void remove_task_tail(struct task_list_item** head, struct task_list_item* item, struct task_list_item** tail)
{
	if(!item)
		return;
	
	if(*head == item)
		*head = item->next;
	
	if(*tail == item) 
  		*tail = item->prev;
}

void delete_task_list_item(struct task_list_item** item)
{
	free((*item)->task->stack);
	free((*item)->task);
	free(*item);
}

struct task *choose_task(void)
{
	scheduler_data.current = scheduler_data.head;
	while(scheduler_data.current)
	{
		if((scheduler_data.current->task->status == TASK_RUNNING) || (scheduler_data.current->task->status == TASK_CREATED)) 
		{
			scheduler_data.current = remove_task_list_item_head(&scheduler_data.head, &scheduler_data.tail);
			insert_task_list_item_tail(&scheduler_data.head, scheduler_data.current, &scheduler_data.tail);

			return scheduler_data.current->task;
		}
	}
	return NULL;
}