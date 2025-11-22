#include <stdlib.h>
#include <stdio.h>
#include "tasklist.h"
#include <ucontext.h>
#include "coroutine.h"
#include <utility>
#include <stdexcept>

struct scheduler_data scheduler_data;
static int id = 1;


void create_task(std::function<void (task &)> func)
{
	
	scheduler_data.task_map.emplace(std::piecewise_construct,
            						std::forward_as_tuple(id),
            						std::forward_as_tuple(func, id));
	id++;
	//scheduler_data.task_list.push_back(std::move(task{func, id++}));
	//scheduler_data.task_list.emplace_back(func, id++);
}


void init_scheduler(std::shared_ptr<EPoller> engine)
{
	scheduler_data.poller = engine;
	//scheduler_data.curr_task = scheduler_data.task_list.end();
}

void run_tasks(void)
{
	while(scheduler_data.task_map.size() > 0)
	{
		for(auto curr_task = scheduler_data.task_map.begin();
				 curr_task != scheduler_data.task_map.end();
		   )
		   {
				if((curr_task->second.get_status() != task_status::TASK_RUNNING) && (curr_task->second.get_status() != task_status::TASK_CREATED))
				{
					curr_task++;
					continue;
				}

				curr_task->second();
				if(curr_task->second.get_status() == TASK_FINISHED)
				{
					curr_task = scheduler_data.task_map.erase(curr_task);
				}
				else
				{
					curr_task++;
				}
		   }

		auto events = scheduler_data.poller->Poll();
        for(const auto& evt:events)
		{
			auto iter = scheduler_data.task_map.find(evt.coro_id);
			if(iter == scheduler_data.task_map.end())
				throw std::runtime_error("coro absent in scheduler map");
	
			// evt.coro_id
			if(evt.command == DescriptorOperations::Error)
			{
				std::runtime_error specific_error("Это сообщение об ошибке, созданное без throw.");
        		std::exception_ptr exptr = std::make_exception_ptr(specific_error);
				iter->second.set_exception(exptr);
				continue;
			}
			iter->second.resume();

		}
		
	}
	std::cout << "Finished run_tasks!" << std::endl;
}

/*
void run_tasks(void)
{
	while(scheduler_data.task_list.size() > 0)
	{
		for(scheduler_data.curr_task = scheduler_data.task_list.begin();
			scheduler_data.curr_task != scheduler_data.task_list.end();
		   )
		   {
				if(((scheduler_data.curr_task)->get_status() != task_status::TASK_RUNNING) && ((scheduler_data.curr_task)->get_status() != task_status::TASK_CREATED))
				{
					scheduler_data.curr_task++;
					continue;
				}

				(*scheduler_data.curr_task)();
				if((scheduler_data.curr_task)->get_status() == TASK_FINISHED)
				{
					scheduler_data.curr_task = scheduler_data.task_list.erase(scheduler_data.curr_task);
				}
				else
				{
					scheduler_data.curr_task++;
				}
		   }
		auto events = scheduler_data.poller->Poll();
        for(const auto& evt:events)
		{
			// evt.coro_id
			// if(evt.command == DescriptorOperations::Error)
		}
		
	}
	std::cout << "Finished run_tasks!" << std::endl;
}
*/	