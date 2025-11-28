#pragma once
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <utility>
#include <vector>
#include "address.h"
#include "coroutine.h"
#include <memory>

class BaseSocket {
public:
	BaseSocket(int domain, int type);
	BaseSocket(int fd);
	int GetFd() const;
protected:
	int SetupNonblockingMode(int s);
	int CreateSocket(int domain, int type);
	void InitFileDescriptor(int s);
	mutable std::shared_ptr<int> fd_ptr;
};

class RWSocket: public BaseSocket {
public:
	RWSocket() : BaseSocket(AF_INET, SOCK_STREAM){}
	RWSocket(SocketAddress address, int fd);
    ssize_t Read(void* buf, size_t count);
	std::string AsyncRead() const;
    ssize_t Write(const void* buf, size_t count);
	void AsyncWrite(const std::string& content, Task& coro) const;
protected:    
	SocketAddress address;
};

class BaseServerSocket: public BaseSocket {
public:
	BaseServerSocket(): BaseSocket(AF_INET, SOCK_STREAM) {}
    void Bind(SocketAddress address);
    void Listen(int backlog = 128);

	std::vector<RWSocket> AsyncAccept() const;
private:
	SocketAddress GetAddress() { return address;}
	SocketAddress address;
};


class ClientSocket: public RWSocket {
public:
    void Connect(SocketAddress address);
private:
};
