#include "stat.h"
#include <stdlib.h>
#include <stdio.h>
//#include "scheduler.h"
#include <ucontext.h>
//#include "coroutine.h"
#include <signal.h>
#include "stackcontext.h"

struct coro_statistic init_statistick(unsigned int coro_id, float max_stack_usage) 
{
    // Инициализация через литерал структуры (C99+)
    struct coro_statistic stat = { .coro_id = coro_id, .max_stack_usage = max_stack_usage };
    return stat;
}

uint32_t* find_mismatch(void* start_ptr, void* end_ptr, uint32_t pattern) {
    uint32_t* current = (uint32_t*)start_ptr;
    uint32_t* end = (uint32_t*)end_ptr;

    while (current < end) {
        if (*current != pattern) {
            return current; // Нашли первое несовпадение
        }
        current++; // Сдвигаемся на sizeof(uint32_t) байт
    }
    return NULL; // Все элементы соответствуют шаблону
}

float get_stack_usage_percent(const struct stack_context* stack, uint32_t pattern)
{
	const uint8_t* stack_low = (uint8_t*) stack->top;
	const uint8_t* stack_high = stack_low + stack->size;

	uint32_t* mismatch= find_mismatch(stack_low, stack_high, pattern);
	if(!mismatch)
		return 100.0f;

	size_t num_bytes = mismatch - (uint32_t*)stack_low;

	uint32_t used = stack->size - num_bytes;
	float res = (float)used * 100 / stack->size;
	return res;
}

/*
float GetStackMaxPercentUsage(struct coro_statistic* stat, uint32_t pattern)
{
	if(ctx.uc_stack.ss_size == 0)
		return 0.0;

	const uint32_t* stack_low = reinterpret_cast<uint32_t*>(ctx.uc_stack.ss_sp);
	const uint32_t* stack_high = stack_low + ctx.uc_stack.ss_size/sizeof(pattern);
    
	auto it = std::find_if(stack_low, stack_high, [&pattern](uint32_t value) {
        return value != pattern;
    });

	if(it == stack_high)
		return 0.0f;

	auto num_bytes = std::distance(stack_low, it) * sizeof(pattern);
	auto used = ctx.uc_stack.ss_size - num_bytes;
	return static_cast<float>(used) * 100.0 / ctx.uc_stack.ss_size;
	// float not_used = static_cast<float>(num_bytes)/ctx.uc_stack.ss_size;
}

float GetStackMaxPercentUsageEx(struct coro_statistic* stat, uint8_t pattern)
{
	if(ctx.uc_stack.ss_size == 0)
		return 0.0;

	const uint8_t* stack_low = reinterpret_cast<uint8_t*>(ctx.uc_stack.ss_sp);
	const uint8_t* stack_high = stack_low + ctx.uc_stack.ss_size/sizeof(pattern);
    
	auto it = std::find_if(stack_low, stack_high, [&pattern](uint8_t value) {
        return value != pattern;
    });

	if(it == stack_high)
		return 0.0f;

	auto num_bytes = std::distance(stack_low, it) * sizeof(pattern);
	auto used = ctx.uc_stack.ss_size - num_bytes;
	return static_cast<float>(used) * 100.0 / ctx.uc_stack.ss_size;
}
*/
//////////////////////////////////////////////////////////////////////////////////
/*
uint32_t* find_mismatch(void* start_ptr, void* end_ptr, uint32_t pattern) {
    uint32_t* current = static_cast<uint32_t*>(start_ptr);
    uint32_t* end = static_cast<uint32_t*>(end_ptr);

    while (current < end) {
        if (*current != pattern) {
            return current; // Нашли первое несовпадение
        }
        current++; // Сдвигаемся на sizeof(uint32_t) байт
    }
    return nullptr; // Все элементы соответствуют шаблону
}
*/
/*
void CoroMonitor::CalcStackUsagePercent()
{
	for(const auto& task_pair : scheduler->task_map) 
	{
		const auto& task = task_pair.second;
		if(task.GetStatus() != task_status::Task_WAITING)
			continue;
		registry[task.GetTaskName()] = CoroStatistic{GetStackCurrentPercentUsage(task.GetContext(), task.GetStackInfo()), 0.0f};
	}
}

void CoroMonitor::PrintCurrentStackUsage()
{
	std::cout << std::fixed << std::setprecision(2);
	for(const auto& [key, value] : registry)
	{
		std::cout << "coro: " << key << ": " << "usage: " << value.cur_stack_usage << "%" << std::endl;
	}

	std::cout << std::defaultfloat;
}

void CoroMonitor::CalcStackUsageMaxPercent()
{
	for(const auto& task_pair : scheduler->task_map) 
	{
		const auto& task = task_pair.second;

		auto it = registry.find(task.GetTaskName());
		if(it == registry.end())
			registry[task.GetTaskName()] = CoroStatistic{0.0f, GetStackMaxPercentUsageEx(task.GetContext(),0xAA)};
		else
			// it->second.max_stack_usage = GetStackMaxPercentUsage(task.GetStackInfo(),0xA5A5);
			
			//it->second.max_stack_usage = GetStackMaxPercentUsage(task.GetContext(),0xDEADBEEF);
			registry[task.GetTaskName()] = CoroStatistic{0.0f, GetStackMaxPercentUsageEx(task.GetContext(),0xAA)};
	}
}

void CoroMonitor::PrintMaxStackUsage()
{
	std::cout << std::fixed << std::setprecision(2);
	for(const auto& [key, value] : registry)
	{
		if(value.max_stack_usage < 0.0)
			continue;
		std::cout << "coro: " << key << ": " << "max usage: " << value.max_stack_usage << std::endl;
	}

	std::cout << std::defaultfloat;
}

void CoroMonitor::onCoroWaiting()
{
	CalcStackUsagePercent();
	PrintCurrentStackUsage();
}

void CoroMonitor::onCoroCompleted()
{
	CalcStackUsageMaxPercent();
	PrintMaxStackUsage();
}
*/