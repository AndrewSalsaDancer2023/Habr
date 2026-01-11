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

#include <netinet/in.h>

int port = 8080;
std::string address{"127.0.0.1"};

std::shared_ptr<EPoller> poller = std::make_shared<EPoller>();
std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>(poller);

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

int main(int argc, char **argv)
{
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