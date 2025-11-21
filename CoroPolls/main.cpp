#include <stdlib.h>
#include <stdio.h>
//#include "address.h"
//#include "basesocket.h"
#include <iostream>
#include <vector>
//#include "epoller.h"
//#include "coroutine.h"
//#include "tasklist.h"
#include "taskscheduler.h"

size_t buffer_size = 512;
int port = 8080;
std::string address{"127.0.0.1"};
/*
void client_handler(EPoller& poller, RWSocket socket, int buffer_size) {
    std::vector<char> buffer(buffer_size); 
    try {
        for( ; ;)
        {
            poller.AddReadEvent(socket.GetFd());
            auto res = socket.AsyncRead(buffer_size);
            //int size = socket.Read(buffer.data(), buffer_size);
            //if(size > 0)
            {
               std::string recv(buffer.data());
               //std::cout << "Received: " << std::string_view(buffer.data(), size) << "\n";
               std::cout << "Received: " << recv << "\n";
            
              poller.AddWriteEvent(socket.GetFd());
              socket.Write(buffer.data(), buffer_size);
              poller.RemoveWriteEvent(socket.GetFd());
              if(recv == "\n")
              {
                std::cout << "Exit" << std::endl;
                return;
              }
            }
        }
        
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
}

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

int main(int argc, char **argv)
{
try {
	init_scheduler();

    int numres = 5;
    create_task({[numres](task & self)
    {
        for (int i = 0; i < numres; ++i)
        {
            
            std::cout << "coroutine " << self.get_id() << ": " << i << std::endl;
            self.yield();
        }
    }});
    
    numres = 10;
    create_task({[numres](task & self)
    {
        for (int i = 0; i < numres; ++i)
        {
            
            std::cout << "coroutine " << self.get_id() << ": " << i << std::endl;
            self.yield();
        }
    }});

	run_tasks();
}
catch(std::exception& ex)
{
    std::cout << ex.what() << std::endl;
}
	return EXIT_SUCCESS;
}

