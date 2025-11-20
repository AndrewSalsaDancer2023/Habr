#include <ucontext.h>
#include <cstdint>
#include <memory>
#include <functional>
 
 //#include <coroutine.h>
#include <iostream>
 
const size_t SIGSTKSZ = 16384;
 
struct coroutine
{
    coroutine(const std::function<void (coroutine &)> func, size_t stack_size = SIGSTKSZ)
        : stack{new unsigned char[stack_size]}, func{func}
    {
        getcontext(&callee);
        callee.uc_link = &caller;
        callee.uc_stack.ss_size = stack_size;
        callee.uc_stack.ss_sp = stack.get();
       // makecontext(&callee, reinterpret_cast<void (*)()>(&coroutine_call), 2, reinterpret_cast<size_t>(this) >> 32, this);
       makecontext(&callee, reinterpret_cast<void (*)()>(&coroutine_call), 1, this);
    }
    coroutine(const coroutine &) = delete;
    coroutine & operator=(const coroutine &) = delete;
    coroutine(coroutine &&) = default;
    coroutine & operator=(coroutine &&) = default;
 
    void operator()()
    {
        if (returned) return;
        swapcontext(&caller, &callee);
    }
 
    operator bool() const
    {
        return !returned;
    }
 
    void yield()
    {
        swapcontext(&callee, &caller);
    }
 
private:
    ucontext_t caller;
    ucontext_t callee;
    std::unique_ptr<unsigned char[]> stack;
    std::function<void (coroutine &)> func;
    bool returned = false;
 
    static void coroutine_call(coroutine* pcoro)
    {
        std::cout << "inside coroutine_call " << std::endl;
        coroutine & this_ = *pcoro;
        this_.func(this_);
        this_.returned = true;
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