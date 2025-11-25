#include "basesocket.h"
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <utility>
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
	InitFileDescriptor(CreateSocket(domain, type));
}

BaseSocket::BaseSocket(int fd)
{
	InitFileDescriptor(SetupNonblockingMode(fd));
}

void BaseSocket::InitFileDescriptor(int s)
{
	fd_ptr = std::shared_ptr<int>(new int(s), [](int* fd_ptr) {
            std::cout << "Закрытие FD: " << *fd_ptr << " через shared_ptr deleter." << std::endl;
            close(*fd_ptr); // Вызов close() при удалении последнего shared_ptr
            delete fd_ptr; // Освобождение памяти, выделенной под сам int*
    });
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
/*
void BaseSocket::Close()
{
	if (file_descriptor >= 0) {
		::close(file_descriptor);
    }
}
	*/

int BaseSocket::GetFd() const
{
    if (fd_ptr)
        return *fd_ptr;

   	return -1;
}
/*
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
*/
void BaseServerSocket::Bind(SocketAddress address)
{
	address_ = std::move(address);
	auto [rawaddr, len] = address_.RawAddr();
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    if (setsockopt(*fd_ptr, SOL_SOCKET, SO_REUSEADDR, (char*) &optval, optlen) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt");
    }
    if (bind(*fd_ptr, rawaddr, len) < 0) {
        throw std::system_error(errno, std::generic_category(), "bind");
    }
	
}

void BaseServerSocket::Listen(int backlog)
{
    if (listen(*fd_ptr, backlog) < 0) {
        throw std::system_error(errno, std::generic_category(), "listen");
    }
}
 
std::vector<RWSocket> BaseServerSocket::AsyncAccept() const
{
	std::vector<RWSocket> res;
	bool hasData = true;
    do
    {
		char clientaddr[sizeof(sockaddr_in)];
    	socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
            
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
        //printf("Принято новое соединение на FD %d\n", conn_sock_fd);
		std::cout << "New connection accepted on FD: %d" << conn_sock_fd << std::endl;
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
	return ::read(*fd_ptr, buf, count); 
}

std::string RWSocket::AsyncRead(size_t buffer_size) const
{
	//std::vector<char> result(buffer_size);
	std::string result;
    char buffer[256];
    ssize_t bytes_read;
	bool hasData = true;

 	do {
        //ssize_t bytes_read = ::read(*fd_ptr, result.data()+offset, buffer_size);
		bytes_read = ::read(*fd_ptr, buffer, sizeof(buffer));


		if (errno == EAGAIN || errno == EWOULDBLOCK) 
			hasData = false;
		else 
			throw std::system_error(errno, std::generic_category(), "async read");

        if (bytes_read == 0) 
            hasData = false;
		else
		{
			result.append(buffer, bytes_read);
/*			if((offset + bytes_read) >= buffer_size)
			{
				
				size_t current_capacity = result.capacity();
    			result.reserve(current_capacity * 2);
				//hasData = false; //add capacity increasing
			}
			else
				offset += bytes_read;*/
		}
	}while(hasData);

	return result;
}

ssize_t RWSocket::Write(const void* buf, size_t count)
{
	return ::write(*fd_ptr, buf, count);
}


//void RWSocket::AsyncWrite(const std::vector<char>& content, task& coro) const
void RWSocket::AsyncWrite(const std::string& content, task& coro) const
{
	if(content.empty())
		return;

	size_t total_size = content.size();
	size_t bytes_sent = 0;

	size_t remaining_bytes = total_size; //total_size - bytes_sent;
	while(remaining_bytes > 0)
	{
       // Попытка отправить оставшиеся данные
       ssize_t sent = write(*fd_ptr, content.data() + bytes_sent, remaining_bytes);

    	if(sent > 0) 
		{
        	// Данные были отправлены успешно (возможно, не все сразу)
        	bytes_sent += sent;
        	// printf("Отправлено %zd байт. Осталось: %zd\n", sent, total_size - bytes_sent);
			std::cout << "Sent: " <<  sent << "remains: " << total_size - bytes_sent << std::endl;
        }
    	else 
		if (sent == -1) 
		{
        	if (errno == EAGAIN || errno == EWOULDBLOCK) {
            	// Буфер сокета заполнился. Это НОРМАЛЬНО.
            	// Мы выходим из обработчика и ждем следующего события EPOLLOUT.
            	//printf("Буфер сокета заполнен. Ожидаем EPOLLOUT.\n");
            	//return;
				coro.yield();
        	} 
			else 
			{
        /*    	perror("write error");
            	close(state->fd); */
				throw std::system_error(errno, std::generic_category(), "async wtite");
        	}
    	}
		remaining_bytes = total_size - bytes_sent;
	}

}

void ConnectToServer(int fd, SocketAddress& address)
{
    auto [rawaddr, len] = address.RawAddr();
    if (connect(fd, rawaddr, len) != 0) 
        throw std::system_error(errno, std::generic_category(), "connect");        

    // return 0;
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
