#include "address.h"
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <utility>
#include <cstring>
#include <stdexcept>


SocketAddress::SocketAddress(const std::string& addr, int port)
{
	if (inet_pton(AF_INET, addr.c_str(), &(address.sin_addr)) <= 0)
 		throw std::runtime_error("Invalid address: '" + addr + "'");

 	address.sin_port = htons(port);
	address.sin_family = AF_INET;

}

SocketAddress::SocketAddress(sockaddr* addr)
{
	sockaddr_in addr4; 
	memcpy(&addr4, addr, sizeof(addr4));
    address = addr4;
}

std::string SocketAddress::ToString() const
{
   char buf[1024];
   const auto* val = &address;

   auto* r = inet_ntop(AF_INET, &val->sin_addr, buf, sizeof(buf));
   if (r) {
            return std::string(r) + ":" + std::to_string(ntohs(val->sin_port));
        }

	return "";
}

std::pair<const sockaddr*, int> SocketAddress::RawAddr() const {
	return {reinterpret_cast<const sockaddr*>(&address), sizeof(sockaddr_in)};
}
