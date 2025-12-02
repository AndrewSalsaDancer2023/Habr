#pragma once
#include <cstdint>
#include <utility> 

uint64_t PackIdFd(int fd, uint32_t id);
std::pair<uint32_t, int> unPackIdFd(uint64_t packed_value);