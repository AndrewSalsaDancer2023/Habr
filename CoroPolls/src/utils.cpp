#include <iostream>
#include <cstdint>
#include <iomanip>
#include <climits>
#include <utility> 

uint64_t PackIdFd(int fd, uint32_t id) 
{
    // Убеждаемся, что тип int имеет размер не менее 32 бит (4 байта * 8 бит/байт)
    static_assert(sizeof(int) * CHAR_BIT >= 32, 
                  "Ошибка компиляции: Тип 'int' слишком мал для хранения 32-битного значения.");

    // Предполагается, что fd не является отрицательным числом,     
    uint64_t id_64 = static_cast<uint64_t>(id);
    uint64_t result = (id_64 << 32);
    uint64_t fd_64 = static_cast<uint64_t>(fd);

    result |= fd_64;

    return result;
}


std::pair<uint32_t, int> unPackIdFd(uint64_t packed_value) {
    uint32_t id = static_cast<uint32_t>(packed_value >> 32);
    int fd = static_cast<int>(packed_value & 0xFFFFFFFF);
    
    return std::make_pair(id, fd);
}