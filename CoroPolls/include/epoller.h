#pragma once
//#include "poller.h"
#include <vector>
#include <map>
#include <sys/epoll.h>
#include <unordered_set>
#include <unordered_map>

const int invalid_handle = -1;
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
    EPoller(std::size_t elements = 1024);
    ~EPoller();
    void AddAcceptEvent(int fd, uint32_t coro_id);
    void AddReadEvent(int fd, uint32_t coro_id);
    void AddWriteEvent(int fd, uint32_t coro_id);
    void AppendWriteEvent(int fd, uint32_t coro_id);
    void AppendReadEvent(int fd, uint32_t coro_id);
	void RemoveWriteEvent(int fd, uint32_t coro_id);
	std::vector<PollResult> Poll(int timeout_milliseconds = 10);
private:
    int FindCoroId(int fd);
    int epoll_descriptor = invalid_handle;
    int accept_descriptor = invalid_handle;

    std::vector<epoll_event> events_hapenned;
};

