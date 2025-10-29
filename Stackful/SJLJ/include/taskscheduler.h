#pragma once

void init_scheduler(void);

void create_task(void (*func)(void*), void *arg);

void run_tasks(void);

void task_yield(void);

