#include <stdlib.h>
#include <stdio.h>
#include "tasklist.h"
#include <ucontext.h>
#include "coroutine.h"
#include <utility>

#if defined(CORO_USE_VALGRIND)
# include <valgrind/valgrind.h>
#endif

const int default_stack_size = 16 * 1024;
const char* memory_alloc_error_msg = "Unable to allocate memory for stack!";
const char* invalid_task_error_msg = "Task argument must be non null!";

struct scheduler_data scheduler_data;
static int id = 1;


void create_task(std::function<void (task &)> func)
{
/*	
	scheduler_data.task_map.emplace(std::piecewise_construct,
            						std::forward_as_tuple(id++),
            						std::forward_as_tuple(func, id));*/
	//scheduler_data.task_list.push_back(std::move(task{func, id++}));
	scheduler_data.task_list.emplace_back(func, id++);
}


void init_scheduler(void)
{
	//scheduler_data.current = scheduler_data.head = scheduler_data.tail = NULL;
	scheduler_data.curr_task = scheduler_data.task_list.end();
}

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
					scheduler_data.curr_task++;
		   }
		
	}
	printf("Finished run_tasks!\n");
}