#include <stdlib.h>
#include <stdio.h>
#include "scheduler.h"
#include <ucontext.h>
#include "coroutine.h"
#include <utility>
#include <stdexcept>

Scheduler::Scheduler(std::shared_ptr<EPoller> plr)
		  :poller{plr}
{}

void Scheduler::ProcessEvents()
{
	auto events = poller->Poll();
    for(const auto& evt:events)
	{
		auto iter = task_map.find(evt.coro_id);
		if(iter == task_map.end())
			throw std::runtime_error("coro is absent in scheduler map");
	
		if(evt.event == DescriptorEvents::Error)
		{
			std::runtime_error specific_error("poller exception");
        	std::exception_ptr exptr = std::make_exception_ptr(specific_error);
			iter->second.SetException(exptr);
		}
		iter->second.AllowResume();
	}

}

void Scheduler::ProcessTasks()
{
	for(auto curr_Task = task_map.begin(); curr_Task != task_map.end(); )
	{
		if((curr_Task->second.GetStatus() != task_status::Task_RUNNING) && (curr_Task->second.GetStatus() != task_status::Task_CREATED))
		{
			curr_Task++;
			continue;
		}

		curr_Task->second();
		if(curr_Task->second.GetStatus() == Task_FINISHED)
			curr_Task = task_map.erase(curr_Task);
		else
			curr_Task++;
	}
}

void Scheduler::RunTasks(void)
{
	while(task_map.size() > 0)
	{
		try
		{ 
			ProcessTasks();
			ProcessEvents();
		}
		catch (const std::exception& e)
        {
            std::cerr << "Exception in Scheduler::RunTasks: " << e.what() << std::endl;
		}
		catch (...)
        {
            std::cerr << "Unknown exception in Scheduler::RunTasks !!!" << std::endl;
        }
	}
	std::cout << "Finished RunTasks!" << std::endl;
}
