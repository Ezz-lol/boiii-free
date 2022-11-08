#pragma once
#include <thread>
#include "nt.hpp"

namespace utils::hardware_breakpoint
{
	enum condition
	{
		execute = 0,
		write = 1,
		read_write = 3
	};


	void set_branch_tracing(bool enabled, CONTEXT& context);
	void set_branch_tracing(bool enabled, uint32_t thread_id = GetCurrentThreadId());

	uint32_t activate(uint64_t address, uint32_t length, condition cond, CONTEXT& context);
	uint32_t activate(void* address, uint32_t length, condition cond, uint32_t thread_id = GetCurrentThreadId());
	uint32_t activate(uint64_t address, uint32_t length, condition cond, uint32_t thread_id = GetCurrentThreadId());

	void deactivate_address(uint64_t address, CONTEXT& context);
	void deactivate_address(void* address, uint32_t thread_id = GetCurrentThreadId());
	void deactivate_address(uint64_t address, uint32_t thread_id = GetCurrentThreadId());

	void deactivate(uint32_t index, CONTEXT& context);
	void deactivate(uint32_t index, uint32_t thread_id = GetCurrentThreadId());

	void deactivate_all(CONTEXT& context);
	void deactivate_all(uint32_t thread_id = GetCurrentThreadId());
}
