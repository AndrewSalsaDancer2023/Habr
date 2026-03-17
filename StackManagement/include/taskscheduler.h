#pragma once
#include "stat.h"
#include "stackcontext.h"

void init_scheduler(void);

void create_task(void (*func)(void *), void *arg, struct stack_context stack);

void run_tasks(void (*get_stat_func)(unsigned int task_id));

void task_yield(void);

