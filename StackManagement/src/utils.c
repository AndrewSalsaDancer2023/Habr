#include "utils.h"
#include <unistd.h>


long get_page_size() 
{
    // Рекомендуемый стандарт POSIX для получения размера страницы в байтах
    return sysconf(_SC_PAGESIZE);
}