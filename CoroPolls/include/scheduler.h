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
	void CreateTask(std::function<void (Task &)> func);
	void InitScheduler(std::shared_ptr<EPoller> engine);
	void RunTasks();
private:
	void ProcessEvents();
	std::unordered_map<uint32_t, struct Task> Task_map;
	std::shared_ptr<EPoller> poller;
	uint32_t id = 1;
};