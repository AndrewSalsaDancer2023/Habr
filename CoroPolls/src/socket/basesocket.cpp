#include "basesocket.h"
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <stdexcept>
#include <system_error>
#include <netinet/in.h>

const std::string conn_closed = "connection closed";
const std::string err_read_socket = "error read data from socket";
const std::string err_write_socket = "error write data to socket";

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
        close(*fd_ptr);
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
        //Вызываем accept() на неблокирующем сокете
        int conn_sock_fd = accept(*fd_ptr, reinterpret_cast<sockaddr*>(&clientaddr[0]), &len);
            
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
            
        //Успешно приняли новое соединение.
        //conn_sock_fd — это новый сокет для клиента.
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

ssize_t NonBlockRWSocket::AsyncReadBytes(Task& coro, void* buffer, size_t length) 
{   
    char* buf_ptr = static_cast<char*>(buffer);
    size_t total_received = 0;

    while(total_received < length) 
    {
        ssize_t bytes_recvd = read(*fd_ptr, buf_ptr + total_received, length - total_received);
        
        if(bytes_recvd <= 0)
        {
            if(bytes_recvd == 0)
                throw std::system_error(errno, std::generic_category(), conn_closed);

            if(errno != EAGAIN && errno != EWOULDBLOCK)
                throw std::system_error(errno, std::generic_category(), err_read_socket);
            else
                coro.Yield();
        }
        else
            total_received += bytes_recvd;
    }
    return total_received;
}

std::string NonBlockRWSocket::AsyncRead(Task& coro)
{
    uint32_t length; 
    ssize_t bytes_read = AsyncReadBytes(coro, &length, sizeof(length));
   
    length = ntohl(length);
    //Теперь используем ту же функцию для чтения ровно length байт данных
    std::vector<char> buffer(length);
    AsyncReadBytes(coro, buffer.data(), length);

    return std::string(buffer.data(), length);
}

void NonBlockRWSocket::AsyncWriteBytes(Task& coro, const void* buffer, size_t length) 
{   
    const char* buf_ptr = static_cast<const char*>(buffer);
    size_t total_send = 0;

    while(total_send < length) 
    {
        ssize_t bytes_sent = write(*fd_ptr, buf_ptr + total_send, length - total_send);
        
        if(bytes_sent < 0)
        {
            if(errno != EAGAIN && errno != EWOULDBLOCK)
                throw std::system_error(errno, std::generic_category(), err_write_socket);
            else
                coro.Yield();
        }
        else
            total_send += bytes_sent;
    }
}

void NonBlockRWSocket::AsyncWrite(const std::string& content, Task& coro)
{
	if(content.empty())
		return;

    uint32_t length = htonl(content.size()); // Преобразование в сетевой порядок байтов
    AsyncWriteBytes(coro, &length, sizeof(length)); 
    AsyncWriteBytes(coro, content.data(), content.size());
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

ClientSocket::ClientSocket()
    :Socket(AF_INET, SOCK_STREAM)
{}

void ClientSocket::Connect(SocketAddress& address)
{
	auto [rawaddr, len] = address.RawAddr();
    if (connect(fd, rawaddr, len) != 0) 
    	throw std::system_error(errno, std::generic_category(), "connect");
}

ssize_t ReadBytes(int sockfd, void* buffer, size_t length) 
{    
    char* buf_ptr = static_cast<char*>(buffer);
    size_t total_received = 0;
    while (total_received < length) 
    {
        ssize_t bytes_recvd = read(sockfd, buf_ptr + total_received, length - total_received);
        
        if (bytes_recvd <= 0) 
            throw std::system_error(errno, std::generic_category(), err_read_socket);

        total_received += bytes_recvd;
    }
    return total_received;
}

std::string ClientSocket::ReceiveString()
{
    uint32_t length; 
    // Используем вспомогательную функцию для чтения ровно 4 байтов
    ssize_t bytes_read = ReadBytes(fd, &length, sizeof(length));

    length = ntohl(length);
    std::cout << "received from server:" << length << std::endl;
    // Теперь используем ту же функцию для чтения ровно length байтов данных
    std::vector<char> buffer(length);
    ReadBytes(fd, buffer.data(), length);

    return std::string(buffer.data(), length);
}

void ClientSocket::SendString(const std::string& data)
{
    // Отправляем длину строки (в сетевом порядке байтов)
    uint32_t length = htonl(data.size()); // Преобразование в сетевой порядок байтов
    if (write(fd, &length, sizeof(length)) == -1) 
        throw std::system_error(errno, std::generic_category(), err_write_socket);

    if (write(fd, data.c_str(), data.size()) == -1) 
        throw std::system_error(errno, std::generic_category(), err_write_socket);
}