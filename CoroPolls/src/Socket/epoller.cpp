#include "epoller.h"
#include <system_error>
#include <unistd.h>

const int epoll_flags = EPOLL_CLOEXEC;
const int invalid_handle = -1;
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

EPoller::EPoller()
{
    epoll_descriptor = epoll_create1(epoll_flags);
    if (epoll_descriptor ==  invalid_handle) 
    {
        throw std::system_error(errno, std::generic_category(), "epoll_create1");
    }
    events_hapenned.resize(max_epoll_events);
}

EPoller::~EPoller()
{
    if (epoll_descriptor != invalid_handle) 
        ::close(epoll_descriptor);
}

void EPoller::AddAcceptEvent(int fd)
{
    auto it = registered_events.find(fd);
    if (it != registered_events.end())
    {
        (*it).second.events |= EPOLLIN;

         if (epoll_ctl(epoll_descriptor, EPOLL_CTL_MOD, fd, &(*it).second) < 0)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl");
/*                if (errno == ENOENT) {
                    if (epoll_ctl(Fd_, EPOLL_CTL_ADD, fd, &eev) < 0) {
                        throw std::system_error(errno, std::generic_category(), "epoll_ctl");
                    }
                } else {
                    throw std::system_error(errno, std::generic_category(), "epoll_ctl");
                }*/
    }
    else 
    {
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = fd;

        if(epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, fd, &event) < 0)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl");

        registered_events[fd] = event;
        accept_descriptors.insert(fd);

    }
}


void EPoller::AddReadEvent(int fd)
{
    auto it = registered_events.find(fd);
    if (it != registered_events.end())
    {
        (*it).second.events |= EPOLLIN;

        if (epoll_ctl(epoll_descriptor, EPOLL_CTL_MOD, fd, &(*it).second) < 0)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl");
    }
    else
    {
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = fd;

        if(epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, fd, &event) < 0)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl");
    
        registered_events[fd] = event;
    }
}

void EPoller::AddWriteEvent(int fd)
{
    auto it = registered_events.find(fd);
    if (it != registered_events.end())
    {
        (*it).second.events |= EPOLLOUT;
        if (epoll_ctl(epoll_descriptor, EPOLL_CTL_MOD, fd, &(*it).second) < 0)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl");
    }
    else
    {
        struct epoll_event event;
        event.events = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = fd;
    
        if(epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, fd, &event) < 0)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl");

        registered_events[fd] = event;
    }
}

void EPoller::RemoveWriteEvent(int fd)
{
    auto it = registered_events.find(fd);
    if (it != registered_events.end())
    {
        (*it).second.events &= ~EPOLLOUT;
        if (epoll_ctl(epoll_descriptor, EPOLL_CTL_MOD, fd, &(*it).second) < 0)
            throw std::system_error(errno, std::generic_category(), "epoll_ctl");
    }
}

// if (epoll_ctl(Fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) 
std::vector<PollResult> EPoller::Poll()
{
    std::vector<PollResult> result;
    int nfds;
    if ((nfds = epoll_wait(epoll_descriptor, &events_hapenned[0], events_hapenned.size(), 10)) < 0) 
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

        // A. Проверка на ошибки или отключение
        if (event_flags & (EPOLLERR | EPOLLHUP)) {
            // Ошибка на дескрипторе или удаленный конец закрыл соединение

            epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, fd, NULL); // Удаляем из epoll
            //close(fd); // Закрываем дескриптор
            result.emplace_back(PollResult{fd, DescriptorOperations::Error});
            continue; // Переходим к следующему событию
        }
        else
        // B. Проверка на готовность к чтению (самое распространенное событие)
        if (event_flags & EPOLLIN) {
            // Если это серверный сокет (слушающий), значит пришло новое соединение
            if (accept_descriptors.find(fd) != accept_descriptors.end()) {
                result.emplace_back(PollResult{fd, DescriptorOperations::Accept});
                // Принять новое соединение (и добавить его в epoll_fd)
            } else {
                // Если это клиентский сокет, значит можно читать данные
                // ... Ваш код read() / recv() ...
                result.emplace_back(PollResult{fd, DescriptorOperations::Read});
            }
        }
        else
        // C. Проверка на готовность к записи
        if (event_flags & EPOLLOUT) {
            // Если сокет готов принимать исходящие данные
            result.emplace_back(PollResult{fd, DescriptorOperations::Write});
        }
    }

    return result;
}
/*
void EPoller::Poll(const struct timespec* ts)
{
	while(Ready)
		GetReadyEvents(ts);
}
void EPoller::GetReadyEvents(const struct timespec* ts)
 {
for (const auto& ch : Changes_) {
        int fd = ch.Fd;
        if (!ch.Handle)
		throw std::logic_error("file descriptor handler is missed!");
     //   auto& ev  = InEvents_[fd];
        epoll_event eev = {};
        eev.data.fd = fd;

        if (ch.Type & TEvent::READ) {
                eev.events |= EPOLLIN;
//                ev.Read = ch.Handle;
            }
            if (ch.Type & TEvent::WRITE) {
                eev.events |= EPOLLOUT;
  //              ev.Write = ch.Handle;
            }
            if (ch.Type & TEvent::RHUP) {
                eev.events |= EPOLLRDHUP;
//                ev.RHup = ch.Handle;
            }

       InEvents_[fd] = ch.Handle;

        if (epoll_ctl(Fd_, EPOLL_CTL_ADD, fd, &eev) < 0) {
               throw std::system_error(errno, std::generic_category(), "epoll_ctl");
        }

    }
    
    Reset();
    
    OutEvents_.resize(std::max<size_t>(1, InEvents_.size()));

    int nfds;
    if ((nfds =  epoll_wait(Fd_, &OutEvents_[0], OutEvents_.size(), calc_timeout_milliseconds(ts))) < 0) {
        if (errno == EINTR) {
            return;
        }
        throw std::system_error(errno, std::generic_category(), "epoll_wait");
    }
    
    
    for (int i = 0; i < nfds; ++i) {
        int fd = OutEvents_[i].data.fd;
        auto handler = InEvents_[fd];
        if (OutEvents_[i].events & EPOLLIN) {
            ReadyEvents_.emplace_back(TEvent{fd, TEvent::READ, handler});
        }
        if (OutEvents_[i].events & EPOLLOUT) {
            ReadyEvents_.emplace_back(TEvent{fd, TEvent::WRITE, handler});
        }
        if (OutEvents_[i].events & EPOLLHUP) {
                ReadyEvents_.emplace_back(TEvent{fd, TEvent::RHUP, handler});
        }
        
    }

	
}

epoll_wait(epoll_fd, events, max_events, timeout)
epoll_fd: файловый дескриптор, возвращаемый epoll_create.
events: массив struct epoll_event, куда будут записаны произошедшие события.
max_events: максимальное количество событий, которое может быть возвращено за один вызов.
timeout: время ожидания в миллисекундах.
timeout = 10: будет ждать максимум 10 миллисекунд.
timeout = -1: ожидает бесконечно.
timeout = 0: возвращается немедленно (не ждет вообще), если нет доступных событий. 

*/