#pragma once
#include <stdint.h>


struct asm_context_continuation
{
	uint64_t rsp;
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t rbx;
	uint64_t rbp;
};
