#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <list>
#include "coroutine.h"
#include "epoller.h"
#include <memory>


class Scheduler
{
public:
	Scheduler(std::shared_ptr<EPoller> plr);
	void create_task(std::function<void (task &)> func);
	void init_scheduler(std::shared_ptr<EPoller> engine);
	void run_tasks();
private:	
	std::unordered_map<int, struct task> task_map;
	std::shared_ptr<EPoller> poller;
	uint32_t id = 1;
};