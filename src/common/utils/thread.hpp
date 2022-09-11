#pragma once
#include <thread>
#include "nt.hpp"

namespace utils::thread
{
	bool set_name(HANDLE t, const std::string& name);
	bool set_name(DWORD id, const std::string& name);
	bool set_name(std::thread& t, const std::string& name);
	bool set_name(const std::string& name);

	template <typename ...Args>
	std::thread create_named_thread(const std::string& name, Args&&... args)
	{
		auto t = std::thread(std::forward<Args>(args)...);
		set_name(t, name);
		return t;
	}

	class handle
	{
	public:
		handle(const DWORD thread_id, const DWORD access = THREAD_ALL_ACCESS)
			: handle_(OpenThread(access, FALSE, thread_id))
		{
		}

		operator bool() const
		{
			return this->handle_;
		}

		operator HANDLE() const
		{
			return this->handle_;
		}

	private:
		nt::handle<> handle_{};
	};

	std::vector<DWORD> get_thread_ids();
	void for_each_thread(const std::function<void(HANDLE)>& callback, DWORD access = THREAD_ALL_ACCESS);

	void suspend_other_threads();
	void resume_other_threads();
}
