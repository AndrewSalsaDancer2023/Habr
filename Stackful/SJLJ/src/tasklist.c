#include "tasklist.h"
#include <stdlib.h>

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

void insert_task_list_tail(struct task_list** head, struct task_list* task_item, struct task_list** tail)
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

struct task_list* remove_task_list_head(struct task_list** head, struct task_list** tail)
{
	struct task_list *item = NULL;
	if(!(*head))
		return NULL;

	item = *head;	
	if((*head) == (*tail))
		*head = *tail = NULL;
	else
		*head = (*head)->next;

	return item;
}

void remove_task_tail(struct task_list** head, struct task_list* item, struct task_list** tail)
{
	if(!item)
		return;
	
	if(*head == item)
		*head = item->next;
	
	if(*tail == item) 
  		*tail = item->prev;
	
	//item->prev = item->next;
}