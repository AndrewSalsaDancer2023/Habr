#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <list>
#include "coroutine.h"
#include "epoller.h"
#include <memory>

#include <stdlib.h>
#include <utility>
#include <stdexcept>

class Scheduler
{
public:
	Scheduler(std::shared_ptr<EPoller> plr);
	
	template<typename Func>
void CreateTask(Func&& func);
/*{
    task_map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(std::forward<Func>(func), id) 
    );
	id++;
}*/
	void InitScheduler(std::shared_ptr<EPoller> engine);
	void RunTasks();
private:
	void ProcessEvents();
	void ProcessTasks();
	std::unordered_map<uint32_t, Task> task_map;
	std::shared_ptr<EPoller> poller;
	uint32_t id = 1;
};

	template<typename Func>
void Scheduler::CreateTask(Func&& func) 
{
    task_map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(std::forward<Func>(func), id) 
    );
	id++;
}