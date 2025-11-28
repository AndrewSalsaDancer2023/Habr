#include <stdlib.h>
#include <stdio.h>
#include "scheduler.h"
#include <ucontext.h>
#include "coroutine.h"
#include <utility>
#include <stdexcept>

void Scheduler::CreateTask(std::function<void (Task &)> func)
{
	
	Task_map.emplace(std::piecewise_construct,
            		 std::forward_as_tuple(id),
            		 std::forward_as_tuple(func, id));
	id++;
}

Scheduler::Scheduler(std::shared_ptr<EPoller> plr)
		  :poller{plr}
{}

void Scheduler::ProcessEvents()
{
	auto events = poller->Poll();
    for(const auto& evt:events)
	{
		auto iter = Task_map.find(evt.coro_id);
		if(iter == Task_map.end())
			throw std::runtime_error("coro is absent in scheduler map");
	
		if(evt.command == DescriptorOperations::Error)
		{
			std::runtime_error specific_error("poller exception");
        	std::exception_ptr exptr = std::make_exception_ptr(specific_error);
			iter->second.SetException(exptr);
			continue;
		}
		iter->second.AllowResume();
	}

}

void Scheduler::RunTasks(void)
{
	while(Task_map.size() > 0)
	{
		for(auto curr_Task = Task_map.begin();
				 curr_Task != Task_map.end();
		   )
		   {
				if((curr_Task->second.GetStatus() != task_status::Task_RUNNING) && (curr_Task->second.GetStatus() != task_status::Task_CREATED))
				{
					curr_Task++;
					continue;
				}

				curr_Task->second();
				if(curr_Task->second.GetStatus() == Task_FINISHED)
				{
					curr_Task = Task_map.erase(curr_Task);
				}
				else
				{
					curr_Task++;
				}
		   }
		ProcessEvents();
	}
	std::cout << "Finished RunTasks!" << std::endl;
}
