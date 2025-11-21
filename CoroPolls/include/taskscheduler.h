#pragma once
#include "coroutine.h"

extern "C" void init_scheduler(void);

//extern "C" void create_task(void (*func)(void*), void *arg);
extern "C" void create_task(std::function<void (task &)> func);

extern "C" void run_tasks(void);

extern "C" void task_yield(void);

