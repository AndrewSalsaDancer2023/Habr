#include "basesocket.h"
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include "io_util.h"
#include <stdexcept>
#include <system_error>

//* - IPv4: "127.0.0.1:8080"
//int buffer_size = 128;
//TAddress address{"::1", port};
//run<TEPoll>(debug, address, buffer_size);
/*
Poller_.Poll();
        Poller_.WakeupReadyHandles();
*/
BaseSocket::BaseSocket(int domain, int type)
{
	file_descriptor = CreateSocket(domain, type);
}

BaseSocket::BaseSocket(int fd)
{
	file_descriptor = SetupNonblockingMode(fd);
}

int BaseSocket::SetupNonblockingMode(int s)
{
    int value;
    socklen_t len = sizeof(value);
    if (getsockopt(s, SOL_SOCKET, SO_TYPE, (char*) &value, &len) == 0) {
        value = 1;
        if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*) &value, len) < 0) {
                throw std::system_error(errno, std::generic_category(), "setsockopt");
        }
    }

    auto flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl");
    }
    if (fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl");
    }
    return s;
}

int BaseSocket::CreateSocket(int domain, int type)
{
    auto s = socket(domain, type, 0);
    if (s == static_cast<decltype(s)>(-1)) {
        throw std::system_error(errno, std::generic_category(), "socket");
    }
   return SetupNonblockingMode(s);
}

void BaseSocket::Close()
{
	if (file_descriptor >= 0) {
		::close(file_descriptor);
    }
}

BaseSocket& BaseSocket::operator=(BaseSocket&& other) noexcept
{
	if(this == &other)
		return *this;

	file_descriptor = other.file_descriptor;
	other.file_descriptor = -1;
	return *this;
}

BaseSocket::BaseSocket(BaseSocket&& other) noexcept 
{
	file_descriptor = other.file_descriptor;
	other.file_descriptor = -1;
}

void BaseServerSocket::Bind(SocketAddress address)
{
	address_ = std::move(address);
	auto [rawaddr, len] = address_.RawAddr();
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*) &optval, optlen) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt");
    }
    if (bind(file_descriptor, rawaddr, len) < 0) {
        throw std::system_error(errno, std::generic_category(), "bind");
    }
	
}

void BaseServerSocket::Listen(int backlog)
{
    if (listen(file_descriptor, backlog) < 0) {
        throw std::system_error(errno, std::generic_category(), "listen");
    }
}
 
RWSocket BaseServerSocket::Accept()
{
    char clientaddr[sizeof(sockaddr_in)];
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
    
	int clientfd = GetAcceptedClientDescriptor(GetFd(),reinterpret_cast<sockaddr*>(&clientaddr[0]), len);
	if (clientfd < 0) {
		throw std::system_error(errno, std::generic_category(), "accept");
	}

//    char clientaddr[sizeof(sockaddr_in)];
//    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
   return RWSocket(SocketAddress{reinterpret_cast<sockaddr*>(&clientaddr[0])}, clientfd);
}

std::vector<RWSocket> BaseServerSocket::AsyncAccept()
{
	std::vector<RWSocket> res;
	bool hasData = true;
    do
    {
		char clientaddr[sizeof(sockaddr_in)];
    	socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
            
        // Вызываем accept() на неблокирующем сокете
        int conn_sock_fd = ::accept(file_descriptor, reinterpret_cast<sockaddr*>(&clientaddr[0]), &len);
            
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
        printf("Принято новое соединение на FD %d\n", conn_sock_fd);
		res.push_back(std::move(RWSocket(SocketAddress{reinterpret_cast<sockaddr*>(&clientaddr[0])}, conn_sock_fd)));
	}while(hasData);

	return res;
}

RWSocket::RWSocket(SocketAddress address, int fd)
			:BaseSocket(fd)
{
	address_ = std::move(address);
}

ssize_t RWSocket::Read(void* buf, size_t count)
{
	return ::read(file_descriptor, buf, count); 
}

std::vector<char> RWSocket::AsyncRead(size_t buffer_size)
{
	std::vector<char> result(buffer_size);
	size_t offset = 0;
	bool hasData = true;
 	do {
        ssize_t bytes_read = ::read(file_descriptor, result.data()+offset, buffer_size);

		if (errno == EAGAIN || errno == EWOULDBLOCK) 
			hasData = false;
		else 
			throw std::system_error(errno, std::generic_category(), "async read");

        if (bytes_read == 0) 
            hasData = false;
		else
		{
			if((offset + bytes_read) >= buffer_size)
			{
				
				size_t current_capacity = result.capacity();
    			result.reserve(current_capacity * 2);
				//hasData = false; //add capacity increasing
			}
			else
				offset += bytes_read;
		}
	}while(hasData);

	return result;
}

ssize_t RWSocket::Write(const void* buf, size_t count)
{
	return ::write(file_descriptor, buf, count);
}


void RWSocket::AsyncWrite(const std::vector<char>& content)
{
	if(content.empty())
		return;

	size_t total_size = content.size();
	size_t bytes_sent = 0;

	size_t remaining_bytes = total_size - bytes_sent;
	while(remaining_bytes > 0)
	{
       // Попытка отправить оставшиеся данные
       ssize_t sent = write(file_descriptor, content.data() + bytes_sent, remaining_bytes);

    	if(sent > 0) 
		{
        	// Данные были отправлены успешно (возможно, не все сразу)
        	bytes_sent += sent;
        	printf("Отправлено %zd байт. Осталось: %zd\n", sent, total_size - bytes_sent);
        }
    	else 
		if (sent == -1) 
		{
        	if (errno == EAGAIN || errno == EWOULDBLOCK) {
            	// Буфер сокета заполнился. Это НОРМАЛЬНО.
            	// Мы выходим из обработчика и ждем следующего события EPOLLOUT.
            	printf("Буфер сокета заполнен. Ожидаем EPOLLOUT.\n");
            	return;
        	} 
		/*	else 
			{
            	perror("write error");
            	close(state->fd);
        	}*/
    	}
		remaining_bytes = total_size - bytes_sent;
	}

}

void ClientSocket::Connect(SocketAddress address)
{
	address_ = std::move(address);
	//address.RawAddr()
	
	// int ret = GetConnectionResult(GetFd(), address);
    ConnectToServer(GetFd(), address_);
//	int ret = poller->Result();
    // if (ret < 0) {
	// 	throw std::system_error(-ret, std::generic_category(), "connect");
	// }
}
