#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <stdlib.h>
#include "stackcontext.h"


struct coro_statistic
{
	unsigned int coro_id;
	float max_stack_usage;	
};

struct coro_statistic init_statistick(unsigned int coro_id, float max_stack_usage); 
float get_stack_usage_percent(const struct stack_context* stack, uint32_t pattern);

/*
class Scheduler;

class CoroMonitor : public AbstractSubscriber
{
public:	
	CoroMonitor(std::shared_ptr<Scheduler> sched)
	:scheduler{sched}
	{}

	void onCoroWaiting() override;
    void onCoroCompleted() override;
	
	void CalcStackUsagePercent();
	void CalcStackUsageMaxPercent();
	void PrintCurrentStackUsage();
	void PrintMaxStackUsage();
private:
	float GetStackMaxPercentUsage(const ucontext_t& ctx, uint32_t pattern);
	float GetStackCurrentPercentUsage(const ucontext_t& ctx, const stack_context& context); 

	float GetStackMaxPercentUsageEx(const ucontext_t& ctx, uint8_t pattern);
	std::map<std::string, CoroStatistic> registry;
	std::shared_ptr<const Scheduler> scheduler;
};
*/
