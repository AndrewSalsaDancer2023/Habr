#pragma once

#include <setjmp.h>

struct sjlj_continuation
{
    jmp_buf buf;
};
