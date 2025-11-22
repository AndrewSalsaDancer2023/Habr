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
//#include "epoller.h"
#include "coroutine.h"
#include <memory>

class BaseSocket {
public:
	BaseSocket(int domain, int type);
	BaseSocket(int fd);
	BaseSocket(BaseSocket&& other) noexcept;
	BaseSocket& operator=(BaseSocket&& other) noexcept;
	~BaseSocket(){Close();}
	int GetFd() {return file_descriptor;}  
protected:
	int SetupNonblockingMode(int s);
	int CreateSocket(int domain, int type);
	void Close();
    int file_descriptor = -1;	
};

class RWSocket: public BaseSocket {
public:
	RWSocket() : BaseSocket(AF_INET, SOCK_STREAM){}
	RWSocket(SocketAddress address, int fd);
    ssize_t Read(void* buf, size_t count);
	std::vector<char> AsyncRead(size_t buffer_size);
    ssize_t Write(const void* buf, size_t count);
	void AsyncWrite(const std::vector<char>& content, task& coro);
protected:    
	SocketAddress address_;
};

class BaseServerSocket: public BaseSocket {
public:
	BaseServerSocket(): BaseSocket(AF_INET, SOCK_STREAM) {}
    void Bind(SocketAddress address);
    void Listen(int backlog = 128);

    RWSocket Accept();
	std::vector<RWSocket> AsyncAccept();
private:
	SocketAddress GetAddress() { return address_;}
	SocketAddress address_;
};


class ClientSocket: public RWSocket {
public:
    void Connect(SocketAddress address);
private:
};
