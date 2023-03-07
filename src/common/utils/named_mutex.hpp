#pragma once

#include <string>
#include <chrono>

namespace utils
{
	class named_mutex
	{
	public:
		named_mutex(const std::string& name);
		~named_mutex();

		named_mutex(named_mutex&&) = delete;
		named_mutex(const named_mutex&) = delete;
		named_mutex& operator=(named_mutex&&) = delete;
		named_mutex& operator=(const named_mutex&) = delete;

		void lock() const;
		bool try_lock(std::chrono::milliseconds timeout = std::chrono::milliseconds{0}) const;
		void unlock() const noexcept;

	private:
		void* handle_{};
	};
}
