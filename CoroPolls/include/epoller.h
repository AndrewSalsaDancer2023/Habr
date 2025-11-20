#pragma once
//#include "poller.h"
#include <vector>
#include <map>
#include <sys/epoll.h>
#include <unordered_set>
#include <unordered_map>

enum class DescriptorOperations
{
  Read,
  Write,
  Accept,
  Close,
  Error
};

struct PollResult
{
	int file_descriptor;
	DescriptorOperations command;
};

class EPoller
{
public:
    EPoller();
    ~EPoller();
    void AddAcceptEvent(int fd);
    void AddReadEvent(int fd);
    void AddWriteEvent(int fd);
	void RemoveWriteEvent(int fd);
	std::vector<PollResult> Poll();
private:
    int epoll_descriptor;
    std::unordered_set<int> accept_descriptors;
    std::unordered_map<int, epoll_event> registered_events;

    std::vector<epoll_event> events_hapenned;
};

