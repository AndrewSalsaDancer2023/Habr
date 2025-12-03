#include "basesocket.h"
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <stdexcept>
#include <system_error>

NonBlockSocket::NonBlockSocket(int domain, int type)
{
	InitFileDescriptor(CreateSocket(domain, type));
}

NonBlockSocket::NonBlockSocket(int fd)
{
	InitFileDescriptor(SetupNonblockingMode(fd));
}

void NonBlockSocket::InitFileDescriptor(int s)
{
	fd_ptr = std::shared_ptr<int>(new int(s), [](int* fd_ptr) 
	{
            std::cout << "closing fd: " << *fd_ptr << " via shared_ptr deleter." << std::endl;
            close(*fd_ptr); // Вызов close() при удалении последнего shared_ptr
            delete fd_ptr;
    });
}

int NonBlockSocket::SetupNonblockingMode(int s)
{
    int value = 1;
    socklen_t len = sizeof(value);
    
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*) &value, len) < 0) 
        throw std::system_error(errno, std::generic_category(), "setsockopt");    

    auto flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl error getting flags");
    }
    if (fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl error setting flag");
    }
    return s;
}

int NonBlockSocket::CreateSocket(int domain, int type)
{
    int s = socket(domain, type, 0);
    if (s == -1) {
        throw std::system_error(errno, std::generic_category(), "socket");
    }
   return SetupNonblockingMode(s);
}

int NonBlockSocket::GetFd() const
{
    if (fd_ptr)
        return *fd_ptr;

   	return -1;
}

void ServerNonBlockSocket::Bind(SocketAddress address)
{
	address = std::move(address);
	auto [rawaddr, len] = address.RawAddr();
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    if (setsockopt(*fd_ptr, SOL_SOCKET, SO_REUSEADDR, (char*) &optval, optlen) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt");
    }
    if (bind(*fd_ptr, rawaddr, len) < 0) {
        throw std::system_error(errno, std::generic_category(), "bind");
    }
}

void ServerNonBlockSocket::Listen(int backlog)
{
    if (listen(*fd_ptr, backlog) < 0) {
        throw std::system_error(errno, std::generic_category(), "listen");
    }
}
 
std::vector<NonBlockRWSocket> ServerNonBlockSocket::AsyncAccept(Task& coro)
{
	std::vector<NonBlockRWSocket> res;
	char clientaddr[sizeof(sockaddr_in)];
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
	bool hasData = true;
	coro.Yield();
    do
    {            
        // Вызываем accept() на неблокирующем сокете
        int conn_sock_fd = ::accept(*fd_ptr, reinterpret_cast<sockaddr*>(&clientaddr[0]), &len);
            
        if (conn_sock_fd == -1) 
		{
            if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
                // Все ожидающие соединения приняты. Выходим из цикла accept().
				hasData = false;
                break; 
			}
            else
                throw std::system_error(errno, std::generic_category(), "async accept");
        }
            
        // Успешно приняли новое соединение.
        // conn_sock_fd — это новый сокет для клиента.
		std::cout << "New connection accepted on fd: " << conn_sock_fd << std::endl;
		res.emplace_back(SocketAddress{reinterpret_cast<sockaddr*>(&clientaddr[0])}, conn_sock_fd);
	}while(hasData);

	return res;
}

NonBlockRWSocket::NonBlockRWSocket(SocketAddress address, int fd)
			:NonBlockSocket(fd)
{
	address = std::move(address);
}

ssize_t NonBlockRWSocket::Read(void* buf, size_t count)
{
	return ::read(*fd_ptr, buf, count); 
}

std::string NonBlockRWSocket::AsyncRead(Task& coro)
{
	coro.Yield();

	std::string result;
    char buffer[256];
    // ssize_t bytes_read;
	bool hasData = true;

 	do {
		ssize_t bytes_read = ::read(*fd_ptr, buffer, sizeof(buffer));
		if(bytes_read>0)
			result.append(buffer, bytes_read);
		else
		{
			hasData = false;
			if (bytes_read < 0) 
			{
				if (errno != EAGAIN && errno != EWOULDBLOCK)
					throw std::system_error(errno, std::generic_category(), "async read");
			}
		}
	}while(hasData);

	return result;
}

ssize_t NonBlockRWSocket::Write(const void* buf, size_t count)
{
	return ::write(*fd_ptr, buf, count);
}

void NonBlockRWSocket::AsyncWrite(const std::string& content, Task& coro)
{
	if(content.empty())
		return;

	coro.Yield();

	size_t total_size = content.size();
	size_t bytes_sent = 0;

	ssize_t remaining_bytes = total_size;
	while(remaining_bytes > 0)
	{
        // Попытка отправить оставшиеся данные
		ssize_t sent = write(*fd_ptr, content.data() + bytes_sent, remaining_bytes);

    	if(sent >= 0) 
		{
        	// Данные были отправлены успешно (возможно, не все сразу)
        	bytes_sent += sent;
			std::cout << "Sent: " <<  sent << "remains: " << total_size - bytes_sent << std::endl;
        }
    	else
		{
        	if (errno == EAGAIN || errno == EWOULDBLOCK) {
            	// Буфер сокета заполнился. Это НОРМАЛЬНО.
            	// Мы выходим из обработчика и ждем следующего события EPOLLOUT.
				coro.Yield();
        	} 
			else
				throw std::system_error(errno, std::generic_category(), "async write");
    	}
		remaining_bytes = total_size - bytes_sent;
	}
}

Socket::Socket(int domain, int type)
{
	CreateSocket(domain, type);
}

Socket::~Socket()
{
	close(fd);
}

int Socket::GetFd() const
{
	return fd;
}

void Socket::CreateSocket(int domain, int type)
{
    fd = socket(domain, type, 0);
    if (fd == -1) {
        throw std::system_error(errno, std::generic_category(), "socket");
    }
}

ssize_t Socket::Read(void* buf, size_t count)
{
	return ::read(fd, buf, count); 
}

ssize_t Socket::Write(const void* buf, size_t count)
{
	return ::write(fd, buf, count);
}


ClientSocket::ClientSocket()
    :Socket(AF_INET, SOCK_STREAM)
{}
/*
void ConnectToServer(int fd, SocketAddress& address)
{
    auto [rawaddr, len] = address.RawAddr();
    if (connect(fd, rawaddr, len) != 0) 
        throw std::system_error(errno, std::generic_category(), "connect");
}
*/
void ClientSocket::Connect(SocketAddress& address)
{
/*	address = std::move(address);
    ConnectToServer(GetFd(), address);*/
	auto [rawaddr, len] = address.RawAddr();
    if (connect(fd, rawaddr, len) != 0) 
    	throw std::system_error(errno, std::generic_category(), "connect");
}

void ClientSocket::SendString(const std::string& str)
{
    auto res = Write(str.c_str(), str.length());
    if(res < 0)
        throw std::system_error(errno, std::generic_category(), "Unable write string to socket!");
    if(!res) 
        std::system_error(errno, std::generic_category(), "Server closed connection!");
}

std::string ClientSocket::ReceiveString()
{
    char buffer[256];
	bool hasData = true;
    std::string result;

 	do {
		ssize_t bytes_read = ::read(fd, buffer, sizeof(buffer));
		if(bytes_read>0)
			result.append(buffer, bytes_read);
		else
		{
			hasData = false;
			if (bytes_read < 0) 
			{
				if (errno != EAGAIN && errno != EWOULDBLOCK)
					throw std::system_error(errno, std::generic_category(), "async read");
			}
		}
    }while(hasData);
    
    return result;
}