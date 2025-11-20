#include <stdlib.h>
#include <stdio.h>
#include "taskscheduler.h"
#include "address.h"
#include "basesocket.h"
#include <iostream>
#include <vector>
#include "epoller.h"
#include "coroutine.h"

size_t buffer_size = 512;
int port = 8080;
std::string address{"127.0.0.1"};

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
            /*if(evt.command == DescriptorOperations::Read)
            {

            }*/
        }
        //schedule(events);
//		auto clientSocket = sock.Accept();
//		client_handler(clientSocket, buffer_size);
	}
}
catch(std::exception& ex)
{
    std::cout << ex.what() << std::endl;
}
	return 0;
}

/*
int main(int argc, char **argv)
{
	tsk1.task_id = 1;
	tsk2.task_id = 2;
	
	tsk1.num_iters = 3;
	tsk2.num_iters = 7;

	init_scheduler();

	create_task(fibonacci, (void*)&tsk1);
	create_task(fibonacci, (void*)&tsk2);

	run_tasks();
	return EXIT_SUCCESS;
}
*/
