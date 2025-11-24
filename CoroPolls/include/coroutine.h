#pragma once

#include <ucontext.h>
#include <cstdint>
#include <memory>
#include <functional>
 #include <exception> 
#include <iostream>
 
const size_t SIGSTKSZ = 16384;
 
enum task_status 
{
	TASK_CREATED,
	TASK_RUNNING,
	TASK_WAITING,
	TASK_FINISHED,
};

//template <typename T_Func>
class task
{
public:
  //      task(T_Func&& func_arg, int cor_id, size_t stack_size = SIGSTKSZ)
         task(std::function<void (task &)> func_arg, int cor_id, size_t stack_size = SIGSTKSZ)
        // : stack{new unsigned char[stack_size]}, func{func}, id{cor_id}
        : stack{new unsigned char[stack_size]}, 
          func{std::move(func_arg)}, 
          id{cor_id}
    {
        getcontext(&callee);
        callee.uc_link = &caller;
        callee.uc_stack.ss_size = stack_size;
        callee.uc_stack.ss_sp = stack.get();
       // makecontext(&callee, reinterpret_cast<void (*)()>(&coroutine_call), 2, reinterpret_cast<size_t>(this) >> 32, this);
       makecontext(&callee, reinterpret_cast<void (*)()>(&task_call), 1, this);
    }
    task(const task &) = delete;
    task & operator=(const task &) = delete;
    // task(task &&) = default;
    // task & operator=(task &&) = default;
 
    // ~task()
    // {
    //     std::cout << "inside destructor: " << id << std::endl;
    // }
    task(task&& other) noexcept
        : caller(other.caller), 
          callee(other.callee), 
          stack(std::move(other.stack)), // Перемещаем владение стеком
          func(std::move(other.func)),
          id(other.id),
          status(other.status)
    {
        // После перемещения владения стеком, 
        // необходимо обновить указатель стека в структуре контекста callee
        callee.uc_stack.ss_sp = stack.get();

        // Опустошаем исходный объект, чтобы он был в безопасном состоянии
        other.id = 0;
        other.status = task_status::TASK_FINISHED;
        other.stack.reset();
        other.func = nullptr;
        // other.stack автоматически станет nullptr после std::move

        // Важно: контексты caller и callee в other теперь "сломаны", 
        // но other находится в состоянии TASK_FINISHED, 
        // поэтому его нельзя будет запустить.
    }

    // --- ОПЕРАТОР ПРИСВАИВАНИЯ ПЕРЕМЕЩЕНИЕМ ---
    task& operator=(task&& other) noexcept
    {
        if (this != &other) {
            // Освобождаем текущие ресурсы
            // (unique_ptr<unsigned char[]> stack сделает это автоматически при присваивании)

            // Перемещаем данные
            caller = other.caller;
            callee = other.callee;
            stack = std::move(other.stack); 
            func = std::move(other.func);
            id = other.id;
            status = other.status;

            // Обновляем указатель стека в callee на новое место в памяти
            callee.uc_stack.ss_sp = stack.get();

            // Опустошаем исходный объект
            other.id = 0;
            other.status = task_status::TASK_FINISHED;
            other.stack.reset();
            other.func = nullptr;
        }
        return *this;
    }

    void operator()()
    {
        if (status == task_status::TASK_FINISHED) return;
        swapcontext(&caller, &callee);
    }
 
    operator bool() const
    {
        return status != task_status::TASK_FINISHED;
    }
 
    void yield()
    {
        status = task_status::TASK_WAITING;
        swapcontext(&callee, &caller);
        rethrow_exception();
    }

    task_status get_status()
    {
        return status;
    }

    void resume()
    {
        status = task_status::TASK_RUNNING;
    }

    int get_id()
    { 
        return id; 
    }
	void  set_exception(const std::exception_ptr& e) noexcept
    {
        m_exception = e;
    }
    const std::exception_ptr& get_exception() const noexcept
    {
        return m_exception;
    }

    void rethrow_exception()
    {
        if(!m_exception)
            return;

        std::exception_ptr e = get_exception();
        set_exception(nullptr);
        std::rethrow_exception(e);
    }

private:
    ucontext_t caller;
    ucontext_t callee;
    std::unique_ptr<unsigned char[]> stack;
    std::function<void (task &)> func = nullptr;
    //T_Func func;
    int id = 0;
    task_status status = task_status::TASK_CREATED;
    std::exception_ptr m_exception;

    static void task_call(task* pcoro)
    {
        std::cout << "inside task " << std::endl;
        // coroutine & this_ = *pcoro;
        // this_.func(this_);
        // this_.returned = true;
        pcoro->status = task_status::TASK_RUNNING;
        pcoro->func(*pcoro);
        pcoro->status = task_status::TASK_FINISHED;
    }
};

/*
int main()
{
    int numres = 15;
    coroutine test{[numres](coroutine & self)
    {
        for (int i = 0; i < numres; ++i)
        {
            std::cout << "coroutine " << i << std::endl;
            self.yield();
        }
    }};
    while(test)
    {
        std::cout << "main" << std::endl;
        test();
    }
    return 0;
}
*/