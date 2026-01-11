#include "epoller.h"
#include <system_error>
#include <unistd.h>
#include "utils.h"

const int epoll_flags = EPOLL_CLOEXEC;
const int max_epoll_events = 100;

const std::string epoll_create_error{"epoll_create1 failed"};
const std::string epoll_add_desc_error{"epoll add descriptor failed"};
const std::string epoll_modify_desc_error{"epoll modify descriptor failed"};
const std::string epoll_desc_exists_error{"epoll descriptor already added"};
const std::string epoll_wait_error{"epoll wait error"};

EPoller::EPoller()
{
    epoll_descriptor = epoll_create1(epoll_flags);
    if(epoll_descriptor ==  invalid_handle) 
    {
        throw std::system_error(errno, std::generic_category(), epoll_create_error);
    }
    events_hapenned.resize(max_epoll_events);
}

EPoller::~EPoller()
{
    if(epoll_descriptor != invalid_handle) 
        close(epoll_descriptor);
}

epoll_event CreateReadEvent(int fd, uint32_t coro_id)
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
    event.data.u64 =  PackIdFd(fd, coro_id);

    return event;
}

void EPoller::AddEpollEvent(int fd, epoll_event& event)
{
    if(epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        // if (errno != EEXIST)
            throw std::system_error(errno, std::generic_category(), epoll_add_desc_error);
        // else
            // throw std::system_error(errno, std::generic_category(), epoll_desc_exists_error);
    }
}

void EPoller::AddAcceptEvent(int fd, uint32_t coro_id)
{
    auto event = CreateReadEvent(fd, coro_id);
    accept_descriptor = fd;
    AddEpollEvent(fd, event);
}

epoll_event CreateReadWriteEvents(int fd, uint32_t coro_id)
{
    struct epoll_event event;
    event.events = EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
    event.data.u64 = PackIdFd(fd, coro_id);;

    return event;
}

void EPoller::ModifyEpollEvent(int fd, epoll_event& event)
{
    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_MOD, fd, &event) < 0)
        throw std::system_error(errno, std::generic_category(), epoll_modify_desc_error); 
}

void EPoller::AppendReadEvent(int fd, uint32_t coro_id)
{
    auto event = CreateReadWriteEvents(fd, coro_id);   
    ModifyEpollEvent(fd, event);
}

void EPoller::AddReadEvent(int fd, uint32_t coro_id)
{
    auto event = CreateReadEvent(fd, coro_id);
    AddEpollEvent(fd, event);
}

struct epoll_event CreateWriteEvent(int fd, uint32_t coro_id)
{
    struct epoll_event event;
    event.events = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLET;
    event.data.u64 = PackIdFd(fd, coro_id);

    return event;
}

void EPoller::AppendWriteEvent(int fd, uint32_t coro_id)
{
    auto event = CreateReadWriteEvents(fd, coro_id);
    ModifyEpollEvent(fd, event);
}

void EPoller::AddWriteEvent(int fd, uint32_t coro_id)
{
    auto event = CreateWriteEvent(fd, coro_id);
    AddEpollEvent(fd, event);
}

void EPoller::RemoveWriteEvent(int fd, uint32_t coro_id)
{
    auto event = CreateReadEvent(fd, coro_id);
    ModifyEpollEvent(fd, event);
}

std::vector<PollResult> EPoller::Poll(int timeout_milliseconds)
{
    std::vector<PollResult> result;
    int nfds;
    if ((nfds = epoll_wait(epoll_descriptor, &events_hapenned[0], events_hapenned.size(), timeout_milliseconds)) < 0) 
    {
        if (errno == EINTR) 
        {
            return result;
        }
        throw std::system_error(errno, std::generic_category(), epoll_wait_error);
    }

    for (int i = 0; i < nfds; ++i) 
    {
        auto [coro_id, fd] = unPackIdFd(events_hapenned[i].data.u64);
        uint32_t event_flags = events_hapenned[i].events;

        //Проверка на ошибки или отключение
        if (event_flags & (EPOLLERR | EPOLLHUP)) 
        {
            // Ошибка на дескрипторе или удаленный конец закрыл соединение
            epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, fd, NULL); // Удаляем из epoll
            result.emplace_back(coro_id, DescriptorEvents::Error);
            continue; // Переходим к следующему событию
        }
        //Проверка на готовность к чтению (самое распространенное событие)
        if (event_flags & EPOLLIN) 
        {
            //Если это серверный сокет (слушающий), значит пришло новое соединение
            if (fd == accept_descriptor) 
            {
                //Принять новое соединение (и добавить его в epoll_fd)
                result.emplace_back(coro_id, DescriptorEvents::Accept);
            } 
            else 
            {
                //Это клиентский сокет, значит можно читать данные
                result.emplace_back(coro_id, DescriptorEvents::Read);
            }
        }
        else
        {
            //Проверка на готовность к записи
            if (event_flags & EPOLLOUT)
                //Cокет готов принимать исходящие данные
                result.emplace_back(coro_id, DescriptorEvents::Write);
        }
    }

    return result;
}