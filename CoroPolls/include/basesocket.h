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

class NonBlockSocket {
public:
	NonBlockSocket(int domain, int type);
	NonBlockSocket(int fd);
	int GetFd() const;
protected:
	int SetupNonblockingMode(int s);
	int CreateSocket(int domain, int type);
	void InitFileDescriptor(int s);
	std::shared_ptr<int> fd_ptr;
};

class NonBlockRWSocket: public NonBlockSocket {
public:
	NonBlockRWSocket() : NonBlockSocket(AF_INET, SOCK_STREAM){}
	NonBlockRWSocket(SocketAddress address, int fd);
    // ssize_t Read(void* buf, size_t count);
	std::string AsyncRead(Task& coro);
    // ssize_t Write(const void* buf, size_t count);
	void AsyncWrite(const std::string& content, Task& coro);
protected:
	ssize_t AsyncReadBytes(Task& coro, void* buffer, size_t length);
	void AsyncWriteBytes(Task& coro, const void* buffer, size_t length);
	SocketAddress address;
};

class ServerNonBlockSocket: public NonBlockSocket {
public:
	ServerNonBlockSocket(): NonBlockSocket(AF_INET, SOCK_STREAM) {}
    void Bind(SocketAddress address);
    void Listen(int backlog = 128);

	std::vector<NonBlockRWSocket> AsyncAccept(Task& coro);
private:
	SocketAddress GetAddress() { return address;}
	SocketAddress address;
};

class Socket {
public:
	Socket(int domain, int type);
	int GetFd() const;
    // ssize_t Read(void* buf, size_t count);
    // ssize_t Write(const void* buf, size_t count);
	~Socket();
protected:
	void CreateSocket(int domain, int type);
	int fd;
};

class ClientSocket: public Socket {
public:
	ClientSocket();
    void Connect(SocketAddress& address);
	void SendString(const std::string& str);
	std::string ReceiveString();
};
