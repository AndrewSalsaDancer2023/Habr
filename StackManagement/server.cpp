#include <stdlib.h>
#include <stdio.h>
#include "address.h"
#include "basesocket.h"
#include "epoller.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <time.h>
#include "scheduler.h"
#include "utils.h"

#include <csignal>
#include <cstring>

#include "stackcontext.h"
#include <netinet/in.h>

int port = 8080;
std::string address{"127.0.0.1"};

std::shared_ptr<EPoller> poller = std::make_shared<EPoller>();
std::shared_ptr<basic_stack> stack = std::make_shared<fixedsize_stack>();
std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>(poller, stack);

const std::string echocommand = "echo:";
const std::string timecommand = "time";
const std::string errorinput = "invalid command";

bool startsWithSubstring(const std::string source, const std::string& substr) 
{
    if (source.find(substr) == 0)
        return true;
    
    return false;
}

std::string ExtractStringAfterPrefix(const std::string& input, const std::string& prefix)
{
    if (input.rfind(prefix, 0) == 0) 
        return input.substr(prefix.length());
    
    return "";
}

void stack_overflow() {
    volatile char array[1024 * 1024]; // Выделяем 1 МБ на стеке
    stack_overflow();
}

std::string GetTimeString()
{
    // int *p = nullptr;
    // *p = 10;
    stack_overflow();
    time_t current_time;
    time(&current_time);
    return std::string(ctime(&current_time));
}

void client_handler(Task& coro, NonBlockRWSocket& socket) 
{
    try 
    {
        poller->AddReadEvent(socket.GetFd(), coro.GetId());
        while(true)
        {
            auto input = socket.AsyncRead(coro);
            if(input.empty())
                return;
            
            std::cout << "Received: " << input << std::endl;

            std::string result;

            if(startsWithSubstring(input, timecommand))
                result = GetTimeString();
            else
            {
                if(startsWithSubstring(input, echocommand))
                    result = ExtractStringAfterPrefix(input, echocommand);
                else
                    result = errorinput;
            }
            
            poller->AppendWriteEvent(socket.GetFd(), coro.GetId());
            socket.AsyncWrite(result, coro);
            poller->RemoveWriteEvent(socket.GetFd(), coro.GetId());
        }
    } catch (const std::exception& ex) 
    {
        std::cout << "Exception: " << ex.what() << std::endl;
    }
}

void segfault_handler(int sig) {
    // Внутри обработчика сигналов безопасно использовать только 
    // "async-signal-safe" функции. std::cout формально к ним не относится,
    // поэтому для надежности используем системный вызов write.
    const char* msg = "Segmentation fault error (SIGSEGV) catched!\n";
    write(STDERR_FILENO, msg, strlen(msg));
    
    // После обработки SIGSEGV программа обычно не может продолжать работу,
    // так как состояние памяти нарушено. Завершаем процесс.
    exit(EXIT_FAILURE); 
}
/*
void setup_sig_segv_handler()
{
	struct sigaction sa;
	sa.sa_handler = segfault_handler; // Указываем нашу функцию-обработчик
    sigemptyset(&sa.sa_mask);         // Очищаем маску сигналов
    /*Ядро автоматически перезапускает прерванный системный вызов. 
    Приложению не нужно вручную обрабатывать EINTR, что упрощает логику.*/
    // sa.sa_flags = SA_RESTART;

    // // Регистрируем наш обработчик для SIGSEGV
    // if (sigaction(SIGSEGV, &sa, NULL) == -1) {
    //     perror("Ошибка регистрации обработчика");
    //     exit(1);
    // }
// }

void setup_sig_segv_handler()
{
    struct sigaction sa;
    stack_t ss;

    // 1. Выделяем память под альтернативный стек
    // SIGSTKSZ — стандартный рекомендуемый размер
    ss.ss_sp = malloc(SIGSTKSZ);
    if (ss.ss_sp == NULL) {
        perror("alt stack memory alloc error");
        exit(EXIT_FAILURE);
    }
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;
    // 2. Регистрируем альтернативный стек в системе
    if (sigaltstack(&ss, NULL) == -1) {
        perror("stack handler register error");
        exit(EXIT_FAILURE);
    }

    // 3. Устанавливаем обработчик для SIGSEGV с флагом SA_ONSTACK
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = segfault_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("register sigsegv handler error");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    setup_sig_segv_handler();
    try
    {
        scheduler->CreateTask([](Task& acceptTask)
        {
            try 
            {
	            ServerNonBlockSocket sock;
	            sock.Bind(SocketAddress(address, port));
	            sock.Listen();
        
                poller->AddAcceptEvent(sock.GetFd(), acceptTask.GetId());
    	        while(true) 
                {
                    for(auto&& clientsocket : sock.AsyncAccept(acceptTask))
                    {
                        scheduler->CreateTask([clientsock = std::move(clientsocket)](Task& client_coro) mutable
                        {
                            client_handler(client_coro, clientsock);
                        });
                    }
                }
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;        
            }
        });
        scheduler->RunTasks();
    } catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
	return EXIT_SUCCESS;
}