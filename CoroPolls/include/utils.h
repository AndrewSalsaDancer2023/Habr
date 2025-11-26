#pragma once
#include <cstdint>
#include <utility>

uint64_t pack_id_fd(int fd, uint32_t id);
std::pair<uint32_t, int> unpack_id_fd(uint64_t packed_value);

