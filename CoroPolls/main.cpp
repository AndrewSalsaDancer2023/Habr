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

void client_handler(task& coro, const RWSocket& socket) 
{
    try 
    {
        poller->AddReadEvent(socket.GetFd(), coro.get_id());
        while(true)
        {
            coro.yield();
            auto input = socket.AsyncRead();
            if(input.empty())
                return;
            
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
            
            // poller->AddWriteEvent(socket.GetFd(), coro.get_id());
            poller->AppendWriteEvent(socket.GetFd(), coro.get_id());
            socket.AsyncWrite(result, coro);
            poller->RemoveWriteEvent(socket.GetFd(), coro.get_id());
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        std::cout << "Finish client handler!" << std::endl;
    }
}

int main(int argc, char **argv)
{
 /*   uint32_t my_id = 12345; 
    int my_fd = 42;             
    uint64_t packed_value = pack_id_fd(my_id, my_fd);
    auto [id, fd] = unpack_id_fd(packed_value);
    std::cout << id << std::endl;
    std::cout << fd << std::endl;
    return 0;*/
    try
    {
        // scheduler.init_scheduler(poller);
        scheduler.create_task({[](task& accepttask)
        {
	        BaseServerSocket sock;
	        sock.Bind(SocketAddress(address, port));
	        sock.Listen();
        
            poller->AddAcceptEvent(sock.GetFd(), accepttask.get_id());
	        while(true) 
            {
                accepttask.yield();
                // auto clientSockets = sock.AsyncAccept();
                // for(auto&& clientsocket : clientSockets)
                for(auto&& clientsocket : sock.AsyncAccept())
                {
                    scheduler.create_task([clientsock = std::move(clientsocket)](task& client_coro)
                    {
                        // RWSocket othsocket = std::move(clientsocket);
                        client_handler(client_coro, clientsock);
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