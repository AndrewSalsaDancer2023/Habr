#pragma once
#include <string>
#include <utility>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

class SocketAddress {
public:
	SocketAddress()=default;
    SocketAddress(const std::string& addr, int port);
    SocketAddress(sockaddr* addr);
    std::string ToString() const;
    std::pair<const sockaddr*, int> RawAddr() const;
private:
	struct sockaddr_in Addr_ = {};    
};
