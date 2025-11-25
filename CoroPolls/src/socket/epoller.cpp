#include "epoller.h"
#include <system_error>
#include <unistd.h>

const int epoll_flags = EPOLL_CLOEXEC;
const int max_epoll_events = 100;

/*
int calc_timeout_milliseconds(const struct timespec* ts = nullptr)
{
	int timeout = 0;
	if(!ts)
	return timeout;
	
	timeout  = ts->tv_sec * 1000;
    	timeout += ts->tv_nsec / 1000000;
        
    return timeout;
}

int wait_events(int ephnd, struct epoll_event* events, int maxevents, const struct timespec* ts) {
    int timeout = 0;
    if (ts) {
        timeout  = ts->tv_sec;
        timeout += ts->tv_nsec / 1000000;
    }

    return epoll_wait(ephnd, events, maxevents, timeout);
}
*/

EPoller::EPoller(std::size_t elements)
{
    epoll_descriptor = epoll_create1(epoll_flags);
    if (epoll_descriptor ==  invalid_handle) 
    {
        throw std::system_error(errno, std::generic_category(), "epoll_create1");
    }
    coro_mapping.reserve(elements);
    events_hapenned.resize(max_epoll_events);
    //registered_events.reserve(elements)
}

EPoller::~EPoller()
{
    if (epoll_descriptor != invalid_handle) 
        ::close(epoll_descriptor);
}

struct epoll_event CreateReadEvent(int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
    event.data.fd = fd;

    return event;
}

void EPoller::AddAcceptEvent(int fd, int coro_id)
{
    // struct epoll_event event;
    // event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
    // event.data.fd = fd;
    auto event = CreateReadEvent(fd);
    accept_descriptor = fd;
    if(epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        if (errno != EEXIST)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl add accept");
    }
    coro_mapping[fd] = coro_id;
}


void EPoller::AddReadEvent(int fd, int coro_id)
{
    auto event = CreateReadEvent(fd);

    if(epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        if (errno != EEXIST)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl add accept");
    }
    coro_mapping[fd] = coro_id;
}

struct epoll_event CreateWriteEvent(int fd)
{
    struct epoll_event event;
    event.events = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLET;
    event.data.fd = fd;

    return event;
}

void EPoller::AddWriteEvent(int fd, int coro_id)
{
    auto event = CreateWriteEvent(fd);

    if(epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        if (errno != EEXIST)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl add accept");
    }
    coro_mapping[fd] = coro_id;
}

void EPoller::RemoveWriteEvent(int fd)
{
    auto event = CreateReadEvent(fd);
    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_MOD, fd, &event) < 0)
        throw std::system_error(errno, std::generic_category(), "epoll_ctl");
}

int EPoller::FindCoroId(int fd)
{
    return coro_mapping.at(fd);
}

std::vector<PollResult> EPoller::Poll(int timeout_milliseconds)
{
    std::vector<PollResult> result;
    int nfds;
    if ((nfds = epoll_wait(epoll_descriptor, &events_hapenned[0], events_hapenned.size(), timeout_milliseconds)) < 0) 
    {
        if (errno == EINTR) {
            return result;
        }
        throw std::system_error(errno, std::generic_category(), "epoll_wait");
    }

    for (int i = 0; i < nfds; ++i) 
    {
        int fd = events_hapenned[i].data.fd;
        uint32_t event_flags = events_hapenned[i].events;
        int coro_id = FindCoroId(fd);

        // A. Проверка на ошибки или отключение
        if (event_flags & (EPOLLERR | EPOLLHUP)) {
            // Ошибка на дескрипторе или удаленный конец закрыл соединение
            epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, fd, NULL); // Удаляем из epoll
            //close(fd); // Закрываем дескриптор
            coro_mapping.erase(fd);
            result.emplace_back(coro_id, DescriptorOperations::Error);
            continue; // Переходим к следующему событию
        }
        // B. Проверка на готовность к чтению (самое распространенное событие)
        if (event_flags & EPOLLIN) 
        {
            // Если это серверный сокет (слушающий), значит пришло новое соединение
            if (fd == accept_descriptor) 
            {
                // Принять новое соединение (и добавить его в epoll_fd)
                result.emplace_back(coro_id, DescriptorOperations::Accept);
            } else {
                // Это клиентский сокет, значит можно читать данные
                result.emplace_back(coro_id, DescriptorOperations::Read);
            }
        }
        else
        {
            // C. Проверка на готовность к записи
            if (event_flags & EPOLLOUT)
                // Если сокет готов принимать исходящие данные
                result.emplace_back(coro_id, DescriptorOperations::Write);
        }
    }

    return result;
}

/*
epoll_wait(epoll_fd, events, max_events, timeout)
epoll_fd: файловый дескриптор, возвращаемый epoll_create.
events: массив struct epoll_event, куда будут записаны произошедшие события.
max_events: максимальное количество событий, которое может быть возвращено за один вызов.
timeout: время ожидания в миллисекундах.
timeout = 10: будет ждать максимум 10 миллисекунд.
timeout = -1: ожидает бесконечно.
timeout = 0: возвращается немедленно (не ждет вообще), если нет доступных событий. 

*/