#include <stdlib.h>
#include <stdio.h>
#include "address.h"
#include "basesocket.h"
#include <iostream>
#include <memory>
#include <string>
#include <time.h>

size_t buffer_size = 512;
int port = 8080;
std::string address{"127.0.0.1"};

void handler(ClientSocket &socket)
{
    std::string user_input;
    while(true) 
    {
        std::cout << "Enter command (time or echo: some text)" << std::endl;
        std::cin >> user_input; 

        socket.SendString(user_input);

        auto res = socket.ReceiveString();
        std::cout << "Responce from server: " << res;
    }
}

int main(int argc, char **argv)
{
    try {
    ClientSocket sock;
    SocketAddress addr{address, port};
    sock.Connect(addr);
    handler(sock);
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

	return EXIT_SUCCESS;
}