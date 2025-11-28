#pragma once

#include <vector>
#include <map>
#include <sys/epoll.h>
#include <unordered_set>
#include <unordered_map>

const int invalid_handle = -1;
const int sleep_default = 10;

enum class DescriptorOperations: unsigned char
{
  Accept,
  Read,
  Write,
  Error
};

struct PollResult
{
	int coro_id;
	DescriptorOperations command;
    PollResult(int id, DescriptorOperations op) : coro_id(id), command(op) {}
};

class EPoller
{
public:
    EPoller();
    ~EPoller();
    void AddAcceptEvent(int fd, uint32_t coro_id);
    void AddReadEvent(int fd, uint32_t coro_id);
    void AddWriteEvent(int fd, uint32_t coro_id);
    void AppendWriteEvent(int fd, uint32_t coro_id);
    void AppendReadEvent(int fd, uint32_t coro_id);
	void RemoveWriteEvent(int fd, uint32_t coro_id);
	std::vector<PollResult> Poll(int timeout_milliseconds = sleep_default);
private:
    void AddEpollEvent(int fd, epoll_event& event);
    void ModifyEpollEvent(int fd, epoll_event& event);
    
    int epoll_descriptor = invalid_handle;
    int accept_descriptor = invalid_handle;

    std::vector<epoll_event> events_hapenned;
};

