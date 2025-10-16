#pragma once

#include <ucontext.h>

struct context_continuation
{
    ucontext_t ctx_func;
};
