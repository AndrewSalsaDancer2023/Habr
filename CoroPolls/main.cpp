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

#include "coro1.h"

size_t buffer_size = 512;
int port = 8080;
std::string address{"127.0.0.1"};

std::shared_ptr<EPoller> poller = std::make_shared<EPoller>();
Scheduler scheduler(poller);

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

std::string GetTimeString()
{
    time_t current_time;
    time(&current_time);
    return std::string(ctime(&current_time));
}

void client_handler(Task& coro, const RWSocket& socket) 
{
    try 
    {
        poller->AddReadEvent(socket.GetFd(), coro.GetId());
        while(true)
        {
            coro.Yield();
            auto input = socket.AsyncRead();
            if(input.empty())
                return;
            
            std::cout << "Received: " << input << std::endl;

            if(input == "\n")
            {
                std::cout << "Exit coroutine: " << coro.GetId() << std::endl;
                return;
            }   

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
            
            // poller->AddWriteEvent(socket.GetFd(), coro.GetId());
            poller->AppendWriteEvent(socket.GetFd(), coro.GetId());
            socket.AsyncWrite(result, coro);
            poller->RemoveWriteEvent(socket.GetFd(), coro.GetId());
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        std::cout << "Finish client handler!" << std::endl;
    }
}

int main(int argc, char **argv)
{
    try
    {
        // scheduler.InitScheduler(poller);
        scheduler.CreateTask({[](Task& acceptTask)
        {
	        BaseServerSocket sock;
	        sock.Bind(SocketAddress(address, port));
	        sock.Listen();
        
            poller->AddAcceptEvent(sock.GetFd(), acceptTask.GetId());
	        while(true) 
            {
                acceptTask.Yield();
                // auto clientSockets = sock.AsyncAccept();
                // for(auto&& clientsocket : clientSockets)
                for(auto&& clientsocket : sock.AsyncAccept())
                {
                    scheduler.CreateTask([clientsock = std::move(clientsocket)](Task& client_coro)
                    {
                        // RWSocket othsocket = std::move(clientsocket);
                        client_handler(client_coro, clientsock);
                    });
                }
            }
        }});
        scheduler.RunTasks();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::cout << "Finish accept handler!" << std::endl;
    }
	return EXIT_SUCCESS;
}


/*
int main()
{
    int numres = 15;
    coroutine test{[numres](coroutine & self)
    {
        for (int i = 0; i < numres; ++i)
        {
            std::cout << "coroutine " << i << std::endl;
            self.Yield();
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