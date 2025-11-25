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

size_t buffer_size = 512;
int port = 8080;
std::string address{"127.0.0.1"};

std::shared_ptr<EPoller> poller = std::make_shared<EPoller>();
Scheduler scheduler(poller);
/*
int main() {
try {
    int numres = 15;
    task test{[numres](task & self)
    {
        for (int i = 0; i < numres; ++i)
        {
            std::cout << "coroutine " << i << std::endl;
            self.yield();
        }
    }, 1};
    while(test)
    {
        std::cout << "main" << std::endl;
        test();
    }
    return 0;

    EPoller poller;
	BaseServerSocket sock;
	sock.Bind(SocketAddress(address, port));
	sock.Listen();
    poller.AddAcceptEvent(sock.GetFd());
	while(true) 
    {
        auto events = poller.Poll();
        for(const auto& evt:events) 
        {
            if(evt.command == DescriptorOperations::Accept)
            {
                auto clientSockets = sock.AsyncAccept();
                for(auto&& client : clientSockets)
                {
                    client_handler(poller, std::move(client), buffer_size);
                }
            }
        }

	}
}
catch(std::exception& ex)
{
    std::cout << ex.what() << std::endl;
}
	return 0;
}
*/
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

void client_handler(task& coro, const RWSocket& socket, int buffer_size) 
{
    try 
    {
        poller->AddReadEvent(socket.GetFd(), coro.get_id());
        while(true)
        {
            coro.yield();
            auto input = socket.AsyncRead(buffer_size);
            std::cout << "Received: " << input << std::endl;

            if(input == "\n")
            {
                std::cout << "Exit coroutine: " << coro.get_id() << std::endl;
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
            
            poller->AddWriteEvent(socket.GetFd(), coro.get_id());
            socket.AsyncWrite(result, coro);
            poller->RemoveWriteEvent(socket.GetFd());
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
        // scheduler.init_scheduler(poller);
        scheduler.create_task({[](task& self)
        {
	        BaseServerSocket sock;
	        sock.Bind(SocketAddress(address, port));
	        sock.Listen();
        
            poller->AddAcceptEvent(sock.GetFd(), self.get_id());
	        while(true) 
            {
                self.yield();
                // auto clientSockets = sock.AsyncAccept();
                // for(auto&& clientsocket : clientSockets)
                for(auto&& clientsocket : sock.AsyncAccept())
                {
                    scheduler.create_task([clientsock = std::move(clientsocket)](task& client_coro)
                    {
                        // RWSocket othsocket = std::move(clientsocket);
                        client_handler(client_coro, clientsock, buffer_size);
                    });
                }
            }
        }});
        scheduler.run_tasks();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::cout << "Finish accept handler!" << std::endl;
    }
	return EXIT_SUCCESS;
}