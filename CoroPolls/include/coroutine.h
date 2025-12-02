#pragma once

#include <ucontext.h>
#include <cstdint>
#include <memory>
#include <functional>
#include <exception> 
#include <iostream>
 
enum task_status 
{
	Task_CREATED,
	Task_RUNNING,
	Task_WAITING,
	Task_FINISHED,
};

const size_t DefaultStackSize = 16384;

class Task
{
public:
    Task(std::function<void (Task &)> func_arg, uint32_t cor_id, size_t stack_size = DefaultStackSize)
        :stack{new unsigned char[stack_size]}, 
         func{func_arg}, 
         id{cor_id}
    {
        getcontext(&callee);
        callee.uc_link = &caller;
        callee.uc_stack.ss_size = stack_size;
        callee.uc_stack.ss_sp = stack.get();
        makecontext(&callee, reinterpret_cast<void (*)()>(&TaskCall), 1, this);
    }
    Task(const Task &) = delete;
    Task & operator=(const Task &) = delete;
  
    Task(Task&& other) = delete;
    Task& operator=(Task&& other) = delete;

    void operator()()
    {
        if(status == task_status::Task_FINISHED) 
            return;
        swapcontext(&caller, &callee);
    }
 
    operator bool() const
    {
        return status != task_status::Task_FINISHED;
    }
 
    void Yield()
    {
        status = task_status::Task_WAITING;
        swapcontext(&callee, &caller);
        RethrowException();
    }

    task_status GetStatus()
    {
        return status;
    }

    void AllowResume()
    {
        status = task_status::Task_RUNNING;
    }

    uint32_t GetId()
    { 
        return id; 
    }

	void SetException(const std::exception_ptr& e) noexcept
    {
        exception = e;
    }

    const std::exception_ptr& GetException() const noexcept
    {
        return exception;
    }

    void RethrowException()
    {
        if(!exception)
            return;

        std::exception_ptr e = GetException();
        SetException(nullptr);
        std::rethrow_exception(e);
    }

private:
    ucontext_t caller;
    ucontext_t callee;
    std::unique_ptr<unsigned char[]> stack;
    std::function<void (Task &)> func;
    
    uint32_t id = 0;
    task_status status = task_status::Task_CREATED;
    std::exception_ptr exception;

    static void TaskCall(Task* pcoro)
    {
        std::cout << "inside Task " << std::endl;

        pcoro->status = task_status::Task_RUNNING;
        pcoro->func(*pcoro);
        pcoro->status = task_status::Task_FINISHED;
    }
};