#pragma once
#include "coroutine.h"
#include <memory>
extern "C" void init_scheduler(std::shared_ptr<EPoller> engine);

extern "C" void create_task(std::function<void (task &)> func);

extern "C" void run_tasks(void);

extern "C" void task_yield(void);

