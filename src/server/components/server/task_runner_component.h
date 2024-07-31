#pragma once
#include <cstdint>

class task_runner_component
{
public:
	virtual bool run_all() = 0;
	virtual bool run_one() = 0;

	virtual void run_detached() = 0;
	
private:
	virtual void join() = 0;
};

class scheduled_task_runner_component : public task_runner_component
{
public:
	scheduled_task_runner_component(std::uint8_t thread_count);
	~scheduled_task_runner_component();

private:
};