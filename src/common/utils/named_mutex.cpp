#include "named_mutex.hpp"
#include "nt.hpp"

namespace utils
{
	named_mutex::named_mutex(const std::string& name)
	{
		this->handle_ = CreateMutexA(nullptr, FALSE, name.data());
	}

	named_mutex::~named_mutex()
	{
		if (this->handle_)
		{
			CloseHandle(this->handle_);
		}
	}

	void named_mutex::lock() const
	{
		if (this->handle_)
		{
			WaitForSingleObject(this->handle_, INFINITE);
		}
	}

	bool named_mutex::try_lock(const std::chrono::milliseconds timeout) const
	{
		if (this->handle_)
		{
			return WAIT_OBJECT_0 == WaitForSingleObject(this->handle_, static_cast<DWORD>(timeout.count()));
		}

		return false;
	}

	void named_mutex::unlock() const noexcept
	{
		if (this->handle_)
		{
			ReleaseMutex(this->handle_);
		}
	}
}
